#include<stdbool.h>
#include<stdio.h>
#include"display.h"
#include"gpu.h"
#include"ints.h"
#include"logger.h"
#include"mem.h"
#include"types.h"

#define _CYCLES_PER_SCANLINE 456
#define _MODE_2_BOUNDS       (_CYCLES_PER_SCANLINE - 80)
#define _MODE_3_BOUNDS       (_MODE_2_BOUNDS - 172)

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

#define BGPIAddress 0xFF68 	/* Backgroound Palette Index */
#define BGPDAddress 0xFF69 	/* Background Palette Data */
#define SPIAddress  0xFF6A 	/* Sprite Palette Index */
#define SPDAddress  0xFF6B 	/* Sprite Palette Data */

#define spriteScreenPosX(ScreenX) (SpriteX-8)
#define spriteScreenPosY(ScreenY) (SpriteY-16)

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


static int const g_sprite_width              = 8;
static int g_current_cycles                  = 0;
static int g_sprite_height                   = 0;
static d8  g_window_tile_map_display_address = 0;
static d8  g_bg_window_tile_data_address     = 0;
static d8  g_bg_tile_map_display_address     = 0;


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


static void _gpu_draw_sprites(void)
{
	//TODO
}


static void _gpu_draw_window(void)
{
	//TODO
}


static void _gpu_draw_background(void)
{
	//TODO
}


static void _gpu_draw_scanline(void)
{
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
		//Draw background if enabled
		if(isLCDC0(lcdc)) {
			_gpu_draw_background();
		}

		//Draw window if enabled
		if(isLCDC5(lcdc)) {
			_gpu_draw_window();
		}

		//Draw sprites if enabled
		if(isLCDC1(lcdc)) {
			_gpu_draw_sprites();
		}
	}
}


static void _gpu_update_lcd_status(void)
{
	//Get required registers
	d8 lcdc = mem_read8(LCDCAddress);
	d8 stat = mem_read8(STATAddress);

	//If LCD is disabled, set mode to 1, reset scanline
	if(!isLCDC7(lcdc)) {
		g_current_cycles = 0;
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
	else if(g_current_cycles >= _MODE_2_BOUNDS)
		current_mode = GPU_OAM;
	else if(g_current_cycles >= _MODE_3_BOUNDS)
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
		//TODO: find the address of CGB Background Color Palette Memory.
		//mem_write8(BGPMAddress + BGPI & 0x1F, new_bgpd);

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
		//TODO: find the address of CGB Sprite Color Palette Memory.
		//mem_write8(SPMAddress + SPI & 0x1F, new_spd);

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
		g_current_cycles += cycles_delta;

	if( g_current_cycles >= _CYCLES_PER_SCANLINE ) {
		//Reset our counter
		g_current_cycles -= _CYCLES_PER_SCANLINE;

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
