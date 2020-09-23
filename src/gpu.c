#include<stdbool.h>
#include<stdio.h>
#include"debug.h"
#include"display.h"
#include"gpu.h"
#include"ints.h"
#include"logger.h"
#include"mem_priv.h"
#include"rom.h"
#include"types.h"

#define _CLOCKS_PER_SCANLINE 456
#define _MODE_2_BOUNDS       (_CLOCKS_PER_SCANLINE - 80)
#define _MODE_3_BOUNDS       (_MODE_2_BOUNDS - 172)

#define BGPDefault 0xE4 	/* Default value for BGP, b11100100 */

#define OAMAddress  0xFE00 	/* Sprite Attribute Table / Object Attribute Memory */

#define LCDCAddress 0xFF40 	/* LCD Controller */
#define STATAddress 0xFF41 	/* LCD Controller Status*/
#define SCYAddress  0xFF42 	/* Background Y Scroll position */
#define SCXAddress  0xFF43 	/* Background X Scroll position */
#define LYAddress   0xFF44 	/* LCD Controller Y-Coordinate */
#define LYCAddress  0xFF45 	/* LY Compare */

#define BGPAddress  0xFF47 	/* Background & Window Palette Data */
#define OBP0Address 0xFF48 	/* Object Palette 0 Data */
#define OBP1Address 0xFF49 	/* Object Palette 1 Data */
#define WYAddress   0xFF4A 	/* Window Y Position */
#define WXAddress   0xFF4B 	/* Window X Position */

#define BGPIAddress 0xFF68 	/* Background Palette Index */
#define BGPDAddress 0xFF69 	/* Background Palette Data */
#define SPIAddress  0xFF6A 	/* Sprite Palette Index */
#define SPDAddress  0xFF6B 	/* Sprite Palette Data */

//TODO: Find proper values
#define SPMAddress  0x0 /* CGB Sprite Color Palette Memory */
#define BGPMAddress 0x0 /* CGB Background Color Palette Memory */

#define spriteScreenPosX(SpriteX) (SpriteX-8)
#define spriteScreenPosY(SpriteY) (SpriteY-16)

#define B0 0x01
#define B1 0x02
#define B2 0x04
#define B3 0x08
#define B4 0x10
#define B5 0x20
#define B6 0x40
#define B7 0x80

#define isLCDC0(reg) ((reg & B0) != 0)
#define isLCDC1(reg) ((reg & B1) != 0)
#define isLCDC2(reg) ((reg & B2) != 0)
#define isLCDC3(reg) ((reg & B3) != 0)
#define isLCDC4(reg) ((reg & B4) != 0)
#define isLCDC5(reg) ((reg & B5) != 0)
#define isLCDC6(reg) ((reg & B6) != 0)
#define isLCDC7(reg) ((reg & B7) != 0)


enum gpu_mode {
	GPU_H_BLANK = 0,
	GPU_V_BLANK = 1,
	GPU_OAM     = 2,
	GPU_VRAM    = 3
};


enum gpu_drawing_type {
	BACKGROUND,
	WINDOW,
	SPRITE
};


typedef struct sprite {
	d8   x;
	d8   y;
	d8   tile_number;
	d8   palette_number_cgb;
	d8   vram_bank_number;
	d8   palette_number_gb;
	bool flipped_x;
	bool flipped_y;
	bool has_priority_over_bg_1_3;
} sprite;


typedef struct bg_attr {
	d8   bgp_number;
	d8   vram_bank_number;
	bool flipped_x;
	bool flipped_y;
	bool has_priority_over_oam;
} bg_attr;


static colour const g_gb_black      = {0, 0, 0, false};
static colour const g_gb_dark_gray  = {85, 85, 85, false};
static colour const g_gb_light_gray = {170, 170, 170, false};
static colour const g_gb_white      = {255, 255, 255, false};

static struct {
	u8 lcdc;
	u8 stat;
	u8 scy;
	u8 scx;
	u8 ly;
	u8 lyc;
	u8 bgp;
	u8 obp0;
	u8 obp1;
	u8 wy;
	u8 wx;
	u8 bgpi;
	u8 bgpd;
	u8 spi;
	u8 spd;
} g_gpu_reg = {0};

static u16       g_current_clocks                  = 0;
static u8        g_sprite_height                   = 0;
static u16       g_mode_2_boundary                 = 0;
static u16       g_mode_3_boundary                 = 0;
static a16       g_window_tile_map_display_address = 0;
static a16       g_bg_window_tile_data_address     = 0;
static a16       g_bg_tile_map_display_address     = 0;


static d8 background_palette_memory[64] = {0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static d8 sprite_palette_memory[64];


static void _gpu_error(enum logger_log_type type, char *title, char *message)
{
	char *full_message = logger_get_msg_buffer();
	snprintf(
		full_message,
		LOG_MESSAGE_MAX_SIZE,
		"[GPU MODULE] %s\n",
		message
	);
	logger_log(
		type,
		title,
		full_message
	);
}


static d16 _gpu_read_spm(u8 index)
{
	d16 spm_colour;
	spm_colour = ((sprite_palette_memory[index+1]) << 8) | (sprite_palette_memory[index]);
	return spm_colour;
}


static void _gpu_write_spm(u8 index, d8 spd)
{
	sprite_palette_memory[index] = spd;
}


static d16 _gpu_read_bgpm(u8 index)
{
	d16 bgpm_colour;
	bgpm_colour = ((background_palette_memory[index+1]) << 8) | (background_palette_memory[index]);
	return bgpm_colour;
}


static void _gpu_write_bgpm(u8 index, d8 spd)
{
	background_palette_memory[index] = spd;
}


static colour _gpu_get_colour_cgb_sprite(u8 colour_number, u8 palette_number)
{
	//Setup
	colour found_colour;
	found_colour.a = (colour_number == 0) ? true : false;
	d16 obp = _gpu_read_spm(palette_number * 8 + colour_number * 2);

	//Acquire colour value
	found_colour.r = obp & (B4 | B3 | B2 | B1 | B0);
	found_colour.g = (obp >> 5) & (B4 | B3 | B2 | B1 | B0);
	found_colour.b = (obp >> 10) & (B4 | B3 | B2 | B1 | B0);

	//Translate colour value to a colour variable
	found_colour.r = (found_colour.r << 3) + ( (found_colour.r >> 2) & (B2 | B1 | B0) );
	found_colour.g = (found_colour.g << 3) + ( (found_colour.g >> 2) & (B2 | B1 | B0) );
	found_colour.b = (found_colour.b << 3) + ( (found_colour.b >> 2) & (B2 | B1 | B0) );

	return found_colour;
}


static colour _gpu_get_colour_cgb(u8 colour_number, u8 palette_number)
{
	//Setup
	colour found_colour;
	found_colour.a = false;
	d16 bgp = _gpu_read_bgpm(palette_number * 8 + colour_number * 2);

	//Acquire colour value
	found_colour.r = bgp & (B4 | B3 | B2 | B1 | B0);
	found_colour.g = (bgp >> 5) & (B4 | B3 | B2 | B1 | B0);
	found_colour.b = (bgp >> 10) & (B4 | B3 | B2 | B1 | B0);

	//Translate colour value to a colour variable
	found_colour.r = (found_colour.r << 3) + ( (found_colour.r >> 2) & (B2 | B1 | B0) );
	found_colour.g = (found_colour.g << 3) + ( (found_colour.g >> 2) & (B2 | B1 | B0) );
	found_colour.b = (found_colour.b << 3) + ( (found_colour.b >> 2) & (B2 | B1 | B0) );

	return found_colour;
}


static colour _gpu_get_colour_gb_sprite(u8 colour_number, u8 palette_number)
{
	//Setup
	colour found_colour;
	found_colour.a = (colour_number == 0) ? true : false;;
	u8 obp = 0;
	switch(palette_number) {
	case 0:
		obp = g_gpu_reg.obp0;
		break;
	case 1:
		obp = g_gpu_reg.obp1;
	}

	//Acquire colour value
	obp >>= colour_number * 2;
	obp &= (B0 | B1);

	//Translate colour value to a colour variable
	switch(obp) {
	case 0:
		found_colour = g_gb_white;
		break;
	case 1:
		found_colour = g_gb_light_gray;
		break;
	case 2:
		found_colour = g_gb_dark_gray;
		break;
	case 3:
		found_colour = g_gb_black;
		break;
	}

	return found_colour;
}


static colour _gpu_get_colour_gb(u8 colour_number)
{
	//Setup
	colour found_colour;
	found_colour.a = false;
	u8 bgp = g_gpu_reg.bgp;

	//Acquire colour value
	bgp >>= colour_number * 2;
	bgp &= (B0 | B1);

	//Translate colour value to a colour variable
	switch(bgp) {
	case 0:
		found_colour = g_gb_white;
		break;
	case 1:
		found_colour = g_gb_light_gray;
		break;
	case 2:
		found_colour = g_gb_dark_gray;
		break;
	case 3:
		found_colour = g_gb_black;
		break;
	}

	return found_colour;
}


static colour _gpu_get_colour(u8 colour_number, u8 palette_number, enum gpu_drawing_type type)
{
	colour found_colour = {255, 255, 255, true};
	if(colour_number > 3) {
		_gpu_error(
			LOG_FATAL,
			"INVALID COLOUR",
			"AN INVALID COLOUR HAS BEEN RECEIVED BY _gpu_get_colour()."
		);
		return found_colour;
	}

	if(rom_is_cgb()) {
		if(type == SPRITE) {
			found_colour = _gpu_get_colour_cgb_sprite(colour_number, palette_number);
		} else {
			found_colour = _gpu_get_colour_cgb(colour_number, palette_number);
		}
	} else {
		if(type == SPRITE) {
			found_colour = _gpu_get_colour_gb_sprite(colour_number, palette_number);
		} else {
			found_colour = _gpu_get_colour_gb(colour_number);
		}
	}

	return found_colour;
}


static sprite _gpu_get_sprite(u8 number)
{
	sprite current_sprite;

	a16 address= OAMAddress + number * 4;
	current_sprite.y = spriteScreenPosY( mem_read8(address) );
	address++;
	current_sprite.x = spriteScreenPosX( mem_read8(address) );
	address++;
	current_sprite.tile_number = mem_read8(address);
	address++;
	d8 bit_data = mem_read8(address);
	current_sprite.palette_number_cgb =        bit_data & (B2 | B1 | B0);
	current_sprite.vram_bank_number =         (bit_data & B3) >> 3;
	current_sprite.palette_number_gb =        (bit_data & B4) >> 4;
	current_sprite.flipped_x =                (bit_data & B5) == B5;
	current_sprite.flipped_y =                (bit_data & B6) == B6;
	current_sprite.has_priority_over_bg_1_3 = (bit_data & B7) == B7;

	return current_sprite;
}


//vram_bank_number is 255-nullable
static void _gpu_get_colour_numbers(
	a16 base_address,
	u8 tile_number,
	u8 line_index,
	u8 vram_bank_number,
	bool flip_x,
	u8 dst[8]
)
{
	//Get line
	d8  line_upper, line_lower;
	a16 first_addr = base_address + (tile_number << 4) + 2 * line_index;
	if(vram_bank_number == 255) {
		line_lower = mem_read8(first_addr);
		line_upper = mem_read8(first_addr + 1);
	} else {
		line_lower = mem_vram_read8(
			vram_bank_number,
			first_addr
		);
		line_upper = mem_vram_read8(
			vram_bank_number,
			first_addr + 1
		);
	}

	u8 current_index;
	for(u8 i = 0; i < 8; i++)
	{
		current_index = !flip_x ? 7 - i : i;
		dst[i] = (BV(line_upper, current_index) << 1) | BV(line_lower, current_index);
		/*
		if(current_index < 6) {
			dst[i] = (
				( ( line_upper & (B7 >> current_index) ) << (6 - current_index) )
				| ( ( line_lower & (B7 >> current_index) ) << (7 - current_index) )
			);
		} else if(current_index < 7) {
			dst[i] = (
				( ( line_upper & (B7 >> current_index) ) >> (6 - current_index) )
				| ( ( line_lower & (B7 >> current_index) ) << (current_index - 7) )
			);
		} else {
			dst[i] = (
				( ( line_upper & (B7 >> current_index) ) << (current_index - 6) )
				| ( ( line_lower & (B7 >> current_index) ) << (current_index - 7) )
			);
		}
		*/
	}
}


static void _gpu_put_sprites(
	colour line[160],
	bool bg_bit_7[160],
	bool bg_colour_is_0[160],
	bool always_prioritised
)
{
	//Get up to 10 sprites in current scanline
	u8 ly = g_gpu_reg.ly;
	u8 sprite_index = 0;
	sprite sprites[10];
	sprite current_sprite;
	for(u8 i = 0; i < 40; i++)
	{
		current_sprite = _gpu_get_sprite(i);

		if( ly < current_sprite.y + g_sprite_height && ly >= current_sprite.y ) {
			sprites[sprite_index] = current_sprite;
			sprite_index++;
			if(sprite_index == 10)
				break;
		}
	}

	//Sort based on Z-priority
	if(!rom_is_cgb())
		for(u8 i = 0; i < sprite_index; i++)
		{
			for(s8 j = 1; j < sprite_index - i; j++)
			{
				if(sprites[j-1].x > sprites[j].x) {
					current_sprite = sprites[j-1];
					sprites[j-1]   = sprites[j];
					sprites[j]     = current_sprite;
				}
			}
		}

	//Get colour numbers
	u8 colour_numbers[10][8];
	d8 tile_number;
	d8 line_index;
	for(u8 i = 0; i < sprite_index; i++)
	{
		//Check which line we are getting
		line_index = sprites[i].flipped_y
				? g_sprite_height - 1 - (ly - sprites[i].y)
				: (ly - sprites[i].y)
		;

		//Get tile address
		if(g_sprite_height == 16)
			if(line_index > 8)
				tile_number = sprites[i].tile_number | B0;
			else
				tile_number = sprites[i].tile_number & !B0;
		else
			tile_number = sprites[i].tile_number;

		//Get single sprite colour numbers
		_gpu_get_colour_numbers(
			OAMAddress,
			tile_number,
			line_index,
			rom_is_cgb() ? sprites[i].vram_bank_number : 255,
			sprites[i].flipped_x,
			colour_numbers[i]
		);
	}


	//Set colours on the line
	u8 current_index;
	for(s8 i = sprite_index - 1; i >= 0; i--)
	{
		for(u8 j = 0; j < 8; j++)
		{
			current_index = sprites[i].x + j;
			if(
				always_prioritised
				|| !bg_bit_7[current_index]
				|| bg_colour_is_0[current_index]
				|| ( !bg_colour_is_0[current_index] && sprites[i].has_priority_over_bg_1_3 )
			) {
				line[current_index] = _gpu_get_colour(
					colour_numbers[i][j],
					rom_is_cgb() ? sprites[i].palette_number_cgb : sprites[i].palette_number_gb,
					SPRITE
				);
			}
		}
	}
}


static void _gpu_get_tile_number_attr(
	a16 map_address,
	a16 tile_map_number,
	a16 data_address,
	s16* tile_number,
	bg_attr* tile_attr
)
{
	u8  u_tile_number;
	s8  s_tile_number;
	d8 tile_attr_byte;

	if(rom_is_cgb()) {
		u_tile_number = (u8) mem_vram_read8(0, map_address + tile_map_number);
		s_tile_number = (s8) u_tile_number;
		*tile_number   = (s16) ((data_address == 0x9000) ? s_tile_number : u_tile_number);

		tile_attr_byte = mem_vram_read8(1, map_address + tile_map_number);
		tile_attr->bgp_number            =  tile_attr_byte & (B2 | B1 | B0);
		tile_attr->vram_bank_number      = (tile_attr_byte & (B3)) >> 3;
		tile_attr->flipped_x             = (tile_attr_byte & (B5)) == B5;
		tile_attr->flipped_y             = (tile_attr_byte & (B6)) == B6;
		tile_attr->has_priority_over_oam = (tile_attr_byte & (B7)) == B7;
	} else {
		*tile_number   = (s16) mem_read8(map_address + tile_map_number);
	}
}


static void _gpu_put_window(colour line[160], bool bg_bit_7[160], bool bg_colour_is_0[160])
{
	//Get data
	u8  ly = g_gpu_reg.ly;
	u8  wy = g_gpu_reg.wy;
	s16 wx = g_gpu_reg.wx - 7;
	s16 tile_map_y = ly - wy;
	u8  tile_map_x = (wx < 0) ? 7 : wx;
	u8  tile_map_tile_y = (tile_map_y - tile_map_y % 8) / 8;
	u8  tile_map_tile_x = (tile_map_x - tile_map_x % 8) / 8;

	//Is the window on screen right now?
	if((ly < wy) || (wy > 143) || (wx > 159)) {
		return;
	}

	//Get proper tile from tile map
	u8  tile_map_number;
	s16 tile_number;
	u8  tile_colour_numbers[8];
	bg_attr tile_attr = {0};
	for(u8 i = 0; i < 20; i++)
	{
		//What's our tile map number (index)
		tile_map_number = tile_map_tile_x + tile_map_tile_y * 32;

		//Get tile number and attributes, if able
		_gpu_get_tile_number_attr(
			g_window_tile_map_display_address,
			tile_map_number,
			g_bg_window_tile_data_address,
			&tile_number,
			&tile_attr
		);

		//Acquire colour numbers
		_gpu_get_colour_numbers(
			g_bg_window_tile_data_address,
			tile_number,
			(rom_is_cgb() && tile_attr.flipped_y) ? 7 - (tile_map_y % 8) : tile_map_y % 8,
			rom_is_cgb() ? tile_attr.vram_bank_number : 255,
			tile_attr.flipped_x,
			tile_colour_numbers
		);

		//Set colours on the line
		u8 current_index;
		for(u8 j = 0; j < 8; j++)
		{
			current_index = (wx + i * 8 + j);
			bg_colour_is_0[current_index] = tile_colour_numbers[j] == 0;
			bg_bit_7[current_index]       = rom_is_cgb() ? tile_attr.has_priority_over_oam : false;
			line[current_index] = _gpu_get_colour(
				tile_colour_numbers[j],
				rom_is_cgb() ? tile_attr.bgp_number : 0,
				WINDOW
			);
		}
	}
}


static void _gpu_put_background(colour line[160], bool bg_bit_7[160], bool bg_colour_is_0[160])
{
	//Get data
	u8 ly  = g_gpu_reg.ly;
	u8 scy = g_gpu_reg.scy;
	u8 scx = g_gpu_reg.scx;

	//Get proper tile from tile map
	//u8 tile_map_number;
	s16 tile_number;
	u8 tile_colour_numbers[8];
	bg_attr tile_attr = {0};
	for(u8 i = 0; i < 20; i++)
	{
		int tile_x_pixel = i*8 + scx;
		int tile_y_pixel = (ly + scy)%256;
		int tile_x = tile_x_pixel/8;
		int tile_y = tile_y_pixel/8;
		int bg_idx = tile_x + tile_y*32;

		//What's our tile map number (index)
		//tile_map_number = tile_map_tile_x + tile_map_tile_y * 32;

		//Get tile number and attributes, if able
		_gpu_get_tile_number_attr(
			g_bg_tile_map_display_address,
			bg_idx, //tile_map_number,
			g_bg_window_tile_data_address,
			&tile_number,
			&tile_attr
		);

		//Acquire colour numbers
		_gpu_get_colour_numbers(
			g_bg_window_tile_data_address,
			tile_number,
			(rom_is_cgb() && tile_attr.flipped_y) ? 7 - (tile_y_pixel % 8) : tile_y_pixel % 8,
			rom_is_cgb() ? tile_attr.vram_bank_number : 255,
			tile_attr.flipped_x,
			tile_colour_numbers
		);

		//Set colours on the line
		u8 current_index;
		for(u8 j = 0; j < 8; j++)
		{
			current_index = (scx + i * 8 + j) % 160;
			bg_colour_is_0[current_index] = tile_colour_numbers[j] == 0;
			bg_bit_7[current_index]       = rom_is_cgb() ? tile_attr.has_priority_over_oam : false;
			line[current_index] = _gpu_get_colour(
				tile_colour_numbers[j],
				rom_is_cgb() ? tile_attr.bgp_number : 0,
				BACKGROUND
			);
		}
	}
}


static void _gpu_restart_boundaries(void)
{
	g_mode_2_boundary = _MODE_2_BOUNDS;
	g_mode_3_boundary = _MODE_3_BOUNDS;
}


static void _gpu_draw_scanline(void)
{
	_gpu_restart_boundaries();

	//Get LCD Controller (LCDC) Register
	u8 lcdc = g_gpu_reg.lcdc;

	//Set correct addresses and values
	g_window_tile_map_display_address 	= isLCDC6(lcdc) ?
			0x9C00 : 0x9800;
	g_bg_window_tile_data_address 		= isLCDC4(lcdc) ?
			0x8000 : 0x9000;
	g_bg_tile_map_display_address 		= isLCDC3(lcdc) ?
			0x9C00 : 0x9800;
	g_sprite_height = isLCDC2(lcdc) ? 16 : 8;

	if(isLCDC7(lcdc)) {
		colour line[160];
		bool   bg_colour_is_0[160];
		bool   bg_bit_7[160];

		//Check if the screen is not fully white
		if(!rom_is_cgb() && !isLCDC0(lcdc)) {
			for(u8 i = 0; i < 160; i++)
			{
				line[i]           = g_gb_white;
				bg_colour_is_0[i] = true;
				bg_bit_7[i]       = false;
			}
		} else {
			_gpu_put_background(line, bg_colour_is_0, bg_bit_7);

			//Draw window if enabled
			if(isLCDC5(lcdc)) {
				_gpu_put_window(line, bg_colour_is_0, bg_bit_7);
			}
		}

		//Put sprites if enabled
		if(isLCDC1(lcdc)) {
			_gpu_put_sprites(
				line,
				bg_colour_is_0,
				bg_bit_7,
				rom_is_cgb() && !isLCDC0(lcdc)
			);
		}

		display_draw_line( line, g_gpu_reg.ly );
	}
}


static void _gpu_update_lcd_status(void)
{
	//Get required registers
	u8 lcdc = g_gpu_reg.lcdc;
	u8 stat = g_gpu_reg.stat;

	//If LCD is disabled, set mode to 1, reset scanline
	if(!isLCDC7(lcdc)) {
		g_current_clocks = 0;
		g_gpu_reg.ly = 0;
		stat &= 0xFC;
		stat |= 0x01;
		g_gpu_reg.stat = stat;
		return;
	}

	//Check in which mode are we now
	u8 current_mode;
	u8 hitherto_mode = stat & 0x03;
	u8 ly = g_gpu_reg.ly;
	if(ly >= 144)
		current_mode = GPU_V_BLANK;
	else if(g_current_clocks >= g_mode_2_boundary)
		current_mode = GPU_OAM;
	else if(g_current_clocks >= g_mode_3_boundary)
		current_mode = GPU_VRAM;
	else
		current_mode = GPU_H_BLANK;

	//Change STAT and determine whether interrupt is needed
	bool request_interrupt = 0;
	switch(current_mode) {
	case GPU_H_BLANK:
		stat &= !B0;
		stat &= !B1;
		request_interrupt = (stat & B3) != 0;
		if (current_mode != hitherto_mode)
			mem_h_blank_notify();
		break;
	case GPU_V_BLANK:
		stat |= B0;
		stat &= !B1;
		request_interrupt = (stat & B4) != 0;
		break;
	case GPU_OAM:
		stat &= !B0;
		stat |= B1;
		request_interrupt = (stat & B5) != 0;
		break;
	case GPU_VRAM:
		stat |= B0;
		stat |= B1;
		break;
	}

	//Request an interrupt if we have just changed the mode
	if(request_interrupt && (current_mode != hitherto_mode))
		ints_request(INT_LCDC);

	//Check the coincidence flag
	u8 lyc = g_gpu_reg.lyc;
	if(ly == lyc) {
		stat |= B2;
		if((stat & B6) != 0)
			ints_request(INT_LCDC);
	} else {
		stat &= !B2;
	}

	//Save proper STAT
	g_gpu_reg.stat = stat;
}


static void _gpu_check_uninitialized_palettes(void)
{
	if (rom_get_header()->cgb_mode != NON_CGB_UNINITIALIZED_PALETTES)
		g_gpu_reg.bgp = BGPDefault;
}


u8 gpu_read_bgpi(void)
{
	return g_gpu_reg.bgpi;
}


void gpu_write_bgpi(u8 new_bgpi)
{
	g_gpu_reg.bgpi = new_bgpi;
}


u8 gpu_read_bgpd(void)
{
	//Only accessible during H-BLANK or V-BLANK
	u8 stat = g_gpu_reg.stat;
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK)
		return g_gpu_reg.bgpd;
	else
		_gpu_error(
			LOG_FATAL,
			"GPU SPD",
			"SPD REGISTER ACCESSED OUTSIDE H- OR V-BLANK"
		);

	return -1;
}


void gpu_write_bgpd(u8 new_bgpd)
{
	//Only accessible during H-BLANK or V-BLANK
	u8 stat = g_gpu_reg.stat;
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK) {
		g_gpu_reg.bgpd = new_bgpd;

		//Update BGP
		u8 bgpi = gpu_read_bgpi();
		_gpu_write_bgpm((bgpi & 0x1F), new_bgpd);

		//Increment BGPI if required
		if((bgpi & B7) == B7) {
			d8 new_bgpi = bgpi;
			new_bgpi++;
			new_bgpi &= !B6;
			gpu_write_bgpi(new_bgpi);
		}
	} else {
		_gpu_error(
			LOG_FATAL,
			"GPU SPD",
			"SPD REGISTER ACCESSED OUTSIDE H- OR V-BLANK"
		);
	}
}


u8 gpu_read_spi(void)
{
	return g_gpu_reg.spi;
}


void gpu_write_spi(u8 new_spi)
{
	g_gpu_reg.spi = new_spi;
}


u8 gpu_read_spd(void)
{
	//Only accessible during H-BLANK or V-BLANK
	u8 stat = g_gpu_reg.stat;
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK)
		return g_gpu_reg.spd;
	else
		_gpu_error(
			LOG_FATAL,
			"GPU SPD",
			"SPD REGISTER ACCESSED OUTSIDE H- OR V-BLANK"
		);

	return -1;
}


void gpu_write_spd(u8 new_spd)
{
	//Only accessible during H-BLANK or V-BLANK
	u8 stat = g_gpu_reg.stat;
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK) {
		g_gpu_reg.spd = new_spd;

		//Update SP
		u8 spi = gpu_read_spi();
		_gpu_write_spm((spi & 0x1F), new_spd);

		//Increment BGPI if required
		if((spi & B7) == B7) {
			u8 new_spi = spi;
			new_spi++;
			new_spi &= !B6;
			gpu_write_spi(new_spi);
		}
	} else {
		_gpu_error(
			LOG_FATAL,
			"GPU SPD",
			"SPD REGISTER ACCESSED OUTSIDE H- OR V-BLANK"
		);
	}
}

static u8 _gpu_read_handler(a16 addr)
{
	switch(addr) {
		case LCDCAddress:
			return g_gpu_reg.lcdc;
			break;
		case STATAddress:
			return g_gpu_reg.stat;
			break;
		case SCYAddress:
			return g_gpu_reg.scy;
			break;
		case SCXAddress:
			return g_gpu_reg.scx;
			break;
		case LYAddress:
			return g_gpu_reg.ly;
			break;
		case LYCAddress:
			return g_gpu_reg.lyc;
			break;
		case BGPAddress:
			return g_gpu_reg.bgp;
			break;
		case OBP0Address:
			return g_gpu_reg.obp0;
			break;
		case OBP1Address:
			return g_gpu_reg.obp1;
			break;
		case WYAddress:
			return g_gpu_reg.wy;
			break;
		case WXAddress:
			return g_gpu_reg.wx;
			break;
		case BGPIAddress:
			return g_gpu_reg.bgpi;
			break;
		case BGPDAddress:
			return g_gpu_reg.bgpd;
			break;
		case SPIAddress:
			return g_gpu_reg.spi;
			break;
		case SPDAddress:
			return g_gpu_reg.spd;
			break;
	}

	debug_assert(false, "_gpu_read_handler: invalid address");
	return 0;
}

static void _gpu_write_handler(a16 addr, u8 data)
{
	switch(addr) {
		case LCDCAddress:
			g_gpu_reg.lcdc = data;
			break;
		case STATAddress:
			g_gpu_reg.stat = data;
			break;
		case SCYAddress:
			g_gpu_reg.scy = data;
			break;
		case SCXAddress:
			g_gpu_reg.scx = data;
			break;
		case LYAddress:
			g_gpu_reg.ly = data;
			break;
		case LYCAddress:
			g_gpu_reg.lyc = data;
			break;
		case BGPAddress:
			g_gpu_reg.bgp = data;
			break;
		case OBP0Address:
			g_gpu_reg.obp0 = data;
			break;
		case OBP1Address:
			g_gpu_reg.obp1 = data;
			break;
		case WYAddress:
			g_gpu_reg.wy = data;
			break;
		case WXAddress:
			g_gpu_reg.wx = data;
			break;
		case BGPIAddress:
			gpu_write_bgpi(data);
			g_gpu_reg.bgpd = gpu_read_bgpd();
			break;
		case BGPDAddress:
			gpu_write_bgpd(data);
			//g_gpu_reg.bgpd = data;
			break;
		case SPIAddress:
			gpu_write_spi(data);
			g_gpu_reg.spd = gpu_read_spd();
			break;
		case SPDAddress:
			gpu_write_spd(data);
			//g_gpu_reg.spd = data;
			break;
		default:
			debug_assert(false, "_gpu_write_handler: invalid address");
		}
}

static void _gpu_register_mem_handler(void)
{
	for (a16 addr = LCDCAddress; addr <= LYCAddress; addr++) {
		mem_register_handlers(addr,
				_gpu_read_handler, _gpu_write_handler);
	}

	for (a16 addr = BGPAddress; addr <= WXAddress; addr++) {
		mem_register_handlers(addr,
				_gpu_read_handler, _gpu_write_handler);
	}

	for (a16 addr = BGPIAddress; addr <= SPDAddress; addr++) {
		mem_register_handlers(addr,
				_gpu_read_handler, _gpu_write_handler);
	}

}

void gpu_prepare(char * rom_title)
{
	_gpu_register_mem_handler();

	_gpu_check_uninitialized_palettes();

	_gpu_restart_boundaries();

	display_prepare(1.0 / FRAME_RATE, rom_title);

	g_gpu_reg.lcdc = 0x91;
	g_gpu_reg.stat = 0x85;
}

void gpu_step(int cycles_delta)
{
	//Get LCD Controller (LCDC) Register
	u8 lcdc = g_gpu_reg.lcdc;

	//Update STAT register
	_gpu_update_lcd_status();

	//Update cycles only if LCD is enabled
	if(isLCDC7(lcdc))
		g_current_clocks += (u16)cycles_delta;

	if( g_current_clocks >= _CLOCKS_PER_SCANLINE ) {
		//Reset our counter
		g_current_clocks -= _CLOCKS_PER_SCANLINE;

		//Trigger the V-Blank interrupt if in V-Blank
		//Reset LY when we reach the end
		//Draw the current scanline if neither
		if(g_gpu_reg.ly == 144)
			ints_request(INT_V_BLANK);
		else if(g_gpu_reg.ly > 153)
			g_gpu_reg.ly = 0;

		if(g_gpu_reg.ly <= 144)
			_gpu_draw_scanline();

		//Increment the LY register
		g_gpu_reg.ly++;
	}
}


void gpu_destroy(void)
{
	display_destroy();
}
