#include<stdbool.h>
#include<stdio.h>
#include"display.h"
#include"gpu.h"
#include"ints.h"
#include"logger.h"
#include"mem.h"
#include"types.h"

#define _CLOCKS_PER_SCANLINE 456
#define _MODE_2_BOUNDS       (_CLOCKS_PER_SCANLINE - 80)
#define _MODE_3_BOUNDS       (_MODE_2_BOUNDS - 172)

#define CGBFAddress 0x0143 	/* CGB Flag */

#define BGPDefault 0xE4 	/* Default value for BGP, b11100100 */

#define OAMAddress  0xFE00 	/* Sprite Attribute Table / Object Attribute Memory */

#define LCDCAddress 0xFF40 	/* LCD Controller */
#define STATAddress 0xFF41 	/* LCD Controller Status*/
#define SCYAddress  0xFF42 	/* Background Y Scroll position */
#define SCXAddress  0xFF43 	/* Background X Scroll position */
#define LYAddress   0xFF44 	/* LCD Controller Y-Coordinate */
#define LYCAddress  0xFF45 	/* LY Compare */
#define DMAAddress  0xFF46 	/* DMA Transfer & Start */
#define BGPAddress  0xFF47 	/* Background & Window Palette Data */
#define OBP0Address 0xFF48 	/* Object Palette 0 Data */
#define OBP1Address 0xFF49 	/* Object Palette 1 Data */
#define WYAddress   0xFF4A 	/* Window Y Position */
#define WXAddress   0xFF4B 	/* Window X Position */

#define VBKAddress   0xFF4F /* VRAM Bank */
#define HDMA1Address 0xFF51 /* New DMA Source, High */
#define HDMA2Address 0xFF52 /* New DMA Source, Low */
#define HDMA3Address 0xFF53 /* New DMA Destination, High */
#define HDMA4Address 0xFF54 /* New DMA Destination, Low */
#define HDMA5Address 0xFF55 /* New DMA Length/Mode/Start, High */

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


static int const    g_sprite_width  = 8;
static colour const g_gb_black      = {0, 0, 0, false};
static colour const g_gb_dark_gray  = {85, 85, 85, false};
static colour const g_gb_light_gray = {170, 170, 170, false};
static colour const g_gb_white      = {255, 255, 255, false};


static int       g_current_clocks                  = 0;
static int       g_sprite_height                   = 0;
static int       g_mode_2_boundary                 = 0;
static int       g_mode_3_boundary                 = 0;
static d8        g_window_tile_map_display_address = 0;
static d8        g_bg_window_tile_data_address     = 0;
static d8        g_bg_tile_map_display_address     = 0;
static bool      g_cgb_enabled                     = true;


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


static colour _gpu_get_colour_cgb_sprite(int colour_number, int palette_number)
{
	//Setup
	colour found_colour;
	found_colour.a = (colour_number == 0) ? true : false;
	d16 bgp = mem_read16(SPMAddress + palette_number * 8 + colour_number * 2);

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


static colour _gpu_get_colour_cgb(int colour_number, int palette_number)
{
	//Setup
	colour found_colour;
	found_colour.a = false;
	d16 bgp = mem_read16(BGPMAddress + palette_number * 8 + colour_number * 2);

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


static colour _gpu_get_colour_gb_sprite(int colour_number, int palette_number)
{
	//Setup
	colour found_colour;
	found_colour.a = (colour_number == 0) ? true : false;;
	a16 address;
	switch(palette_number) {
	case 0:
		address = OBP0Address;
		break;
	case 1:
		address = OBP1Address;
	}
	d8 bgp = mem_read8(address);

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


static colour _gpu_get_colour_gb(int colour_number)
{
	//Setup
	colour found_colour;
	found_colour.a = false;
	d8 bgp = mem_read8(BGPAddress);

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


static colour _gpu_get_colour(int colour_number, int palette_number, enum gpu_drawing_type type)
{
	colour found_colour = {255, 255, 255, true};
	if( (colour_number < 0) || (colour_number > 3) ) {
		_gpu_error(
			LOG_FATAL,
			"INVALID COLOUR",
			"AN INVALID COLOUR HAS BEEN RECEIVED BY _gpu_get_colour()."
		);
		return found_colour;
	}

	if(g_cgb_enabled) {
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


static sprite _gpu_get_sprite(int number)
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


static void _gpu_put_sprites(
	colour line[160],
	bool bg_bit_7[160],
	bool bg_colour_is_0[160],
	bool always_prioritised
)
{
	//Get up to 10 sprites in current scanline
	d8 ly = mem_read8(LYAddress);
	int sprite_index = 0;
	sprite sprites[10];
	sprite current_sprite;
	for(int i = 0; i < 40; i++)
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
	if(!g_cgb_enabled) {
		for(int i = 0; i < sprite_index; i++)
		{
			for(int j = 1; j < sprite_index - i; j++)
			{
				if(sprites[j-1].x > sprites[j].x) {
					current_sprite = sprites[j-1];
					sprites[j-1]   = sprites[j];
					sprites[j]     = current_sprite;
				}
			}
		}
	}

	//Get colour numbers
	int colour_numbers[8][10];
	d8 tile_number;
	d8 line_index;
	for(int i = 0; i < sprite_index; i++)
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

		//Get line
		//TODO: VRAM Banking - Issue #52
		a16 tile_address_base;
		if(sprites[i].vram_bank_number == 0)
			tile_address_base = OAMAddress;
		else if(sprites[i].vram_bank_number == 1)
			tile_address_base = OAMAddress;
		d8 line_upper, line_lower;
		line_lower = mem_read8( tile_address_base + tile_number * 2 + ((line_index * 2) % 8 ) );
		line_upper = mem_read8( tile_address_base + tile_number * 2 + ((line_index * 2) % 8 ) + 1 );
		for(int j = 0; j < 8; j++)
		{
			colour_numbers[j][i]
				= ( ( line_upper & (B7 >> j) ) << (j-1) )
				| ( ( line_lower & (B7 >> j) ) << j )
			;
		}
		int swap;
		if(sprites[i].flipped_x)
			for(int j = 0; j < 4; j++)
			{
				swap                   = colour_numbers[j][i];
				colour_numbers[j][i]   = colour_numbers[7-j][i];
				colour_numbers[7-j][i] = swap;
			}
	}


	//Set colours on the line
	int current_index;
	for(int i = sprite_index - 1; i >= 0; i--)
	{
		for(int j = 0; j < 8; j++)
		{
			current_index = sprites[i].x + j;
			if(
				always_prioritised
				|| !bg_bit_7[current_index]
				|| bg_colour_is_0[current_index]
				|| ( !bg_colour_is_0[current_index] && sprites[i].has_priority_over_bg_1_3 )
			) {
				line[current_index] = _gpu_get_colour(
					colour_numbers[j][i],
					g_cgb_enabled ? sprites[i].palette_number_cgb : sprites[i].palette_number_gb,
					SPRITE
				);
			}
		}
	}
}


static void _gpu_draw_window(colour line[160], bool bg_bit_7[160], bool bg_colour_is_0[160])
{
	//TODO
}


static void _gpu_draw_background(colour line[160], bool bg_bit_7[160], bool bg_colour_is_0[160])
{
	//TODO
}


static void _gpu_restart_boundaries()
{
	g_mode_2_boundary = _MODE_2_BOUNDS;
	g_mode_3_boundary = _MODE_3_BOUNDS;
}


static void _gpu_draw_scanline(void)
{
	_gpu_restart_boundaries();

	//Get LCD Controller (LCDC) Register
	d8 lcdc = mem_read8(LCDCAddress);

	//Set correct addresses and values
	g_window_tile_map_display_address 	= isLCDC6(lcdc) ?
			0x9C00 : 0x9800;
	g_bg_window_tile_data_address 		= isLCDC4(lcdc) ?
			0x8000 : 0x8800;
	g_bg_tile_map_display_address 		= isLCDC3(lcdc) ?
			0x9C00 : 0x9800;
	g_sprite_height = isLCDC2(lcdc) ? 16 : 8;

	if(isLCDC7(lcdc)) {
		colour line[160];
		bool   bg_colour_is_0[160];
		bool   bg_bit_7[160];

		//Check if the screen is not fully white
		if(!g_cgb_enabled && !isLCDC0(lcdc)) {
			_gpu_draw_background(line, bg_colour_is_0, bg_bit_7);

			//Draw window if enabled
			if(isLCDC5(lcdc)) {
				_gpu_draw_window(line, bg_colour_is_0, bg_bit_7);
			}
		} else {
			for(int i = 0; i < 160; i++)
				line[i] = g_gb_white;
		}

		//Put sprites if enabled
		if(isLCDC1(lcdc)) {
			_gpu_put_sprites(
				line,
				bg_colour_is_0,
				bg_bit_7,
				g_cgb_enabled && !isLCDC0(lcdc)
			);
		}

		display_draw_line( line, mem_read8(LYAddress) );
	}
}


static void _gpu_update_lcd_status(void)
{
	//Get required registers
	d8 lcdc = mem_read8(LCDCAddress);
	d8 stat = mem_read8(STATAddress);

	//If LCD is disabled, set mode to 1, reset scanline
	if(!isLCDC7(lcdc)) {
		g_current_clocks = 0;
		mem_write8(LYAddress, 0);
		stat &= 0xFC;
		stat |= 0x01;
		mem_write8(STATAddress, stat);
		return;
	}

	//Check in which mode are we now
	int current_mode;
	int hitherto_mode = stat & 0x03;
	d8 ly = mem_read8(LYAddress);
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
	d8 lyc = mem_read8(LYCAddress);
	if(ly == lyc) {
		stat |= B2;
		if((stat & B6) != 0)
			ints_request(INT_LCDC);
	} else {
		stat &= !B2;
	}

	//Save proper STAT
	mem_write8(STATAddress, stat);
}


static void _gpu_check_cgb_flag()
{
	d8 cgb_flag = mem_read8(CGBFAddress);
	//Determine whether CGB functions are enabled
	g_cgb_enabled = (cgb_flag & B7) == B7;

	//Check for special, non-initialised Non-CGB mode
	if( (g_cgb_enabled) && ( (cgb_flag & (B2 | B3) ) != 0 ) )
		g_cgb_enabled = false;
	else
		mem_write8(BGPAddress, BGPDefault);
}


d8 gpu_read_bgpi()
{
	return mem_read8(BGPIAddress);
}


void gpu_write_bgpi(d8 new_bgpi)
{
	mem_write8(BGPIAddress, new_bgpi);
}


d8 gpu_read_bgpd()
{
	//Only accessible during H-BLANK or V-BLANK
	d8 stat = mem_read8(STATAddress);
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK)
		return mem_read8(BGPDAddress);
	else
		_gpu_error(
			LOG_FATAL,
			"GPU SPD",
			"SPD REGISTER ACCESSED OUTSIDE H- OR V-BLANK"
		);

	return -1;
}


void gpu_write_bgpd(d8 new_bgpd)
{
	//Only accessible during H-BLANK or V-BLANK
	d8 stat = mem_read8(STATAddress);
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK) {
		mem_write8(BGPDAddress, new_bgpd);

		//Update BGP
		d8 bgpi = gpu_read_bgpi();
		mem_write8(BGPMAddress + (bgpi & 0x1F), new_bgpd);

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


d8 gpu_read_spi()
{
	return mem_read8(SPIAddress);
}


void gpu_write_spi(d8 new_spi)
{
	mem_write8(SPIAddress, new_spi);
}


d8 gpu_read_spd()
{
	//Only accessible during H-BLANK or V-BLANK
	d8 stat = mem_read8(STATAddress);
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK)
		return mem_read8(SPDAddress);
	else
		_gpu_error(
			LOG_FATAL,
			"GPU SPD",
			"SPD REGISTER ACCESSED OUTSIDE H- OR V-BLANK"
		);

	return -1;
}


void gpu_write_spd(d8 new_spd)
{
	//Only accessible during H-BLANK or V-BLANK
	d8 stat = mem_read8(STATAddress);
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK) {
		mem_write8(SPDAddress, new_spd);

		//Update SP
		d8 spi = gpu_read_spi();
		mem_write8(SPMAddress + (spi & 0x1F), new_spd);

		//Increment BGPI if required
		if((spi & B7) == B7) {
			d8 new_spi = spi;
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


void gpu_prepare(char * rom_title)
{
	_gpu_check_cgb_flag();

	_gpu_restart_boundaries();

	display_prepare(1.0 / FRAME_RATE, rom_title);
}

void gpu_step(int cycles_delta)
{
	//Get LCD Controller (LCDC) Register
	d8 lcdc = mem_read8(LCDCAddress);

	//Update STAT register
	_gpu_update_lcd_status();

	//Update cycles only if LCD is enabled
	if(isLCDC7(lcdc))
		g_current_clocks += cycles_delta;

	if( g_current_clocks >= _CLOCKS_PER_SCANLINE ) {
		//Reset our counter
		g_current_clocks -= _CLOCKS_PER_SCANLINE;

		//Increment the LY register
		d8 ly = mem_read8(LYAddress);
		ly++;
		mem_write8(LYAddress, ly);

		//Trigger the V-Blank interrupt if in V-Blank
		//Reset LY when we reach the end
		//Draw the current scanline if neither
		if(ly == 144)
			ints_request(INT_V_BLANK);
		else if(ly > 153)
			mem_write8(LYAddress, 0);
		else
			_gpu_draw_scanline();
	}
}


void gpu_destroy(void)
{
    display_destroy();
}
