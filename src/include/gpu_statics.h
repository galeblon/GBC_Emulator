#ifndef GPU_STATICS_H_
#define GPU_STATICS_H_


#include"types.h"


#define _CLOCKS_PER_SCANLINE 456

#define MODE_0_CLOCKS		204
#define MODE_1_CLOCKS		4560
#define MODE_2_CLOCKS		80
#define MODE_3_CLOCKS		172

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


#define g_cgb_000000 {0x00, 0x00, 0x00, false}
#define g_cgb_0000ff {0x00, 0x00, 0xFF, false}
#define g_cgb_adad84 {0xAD, 0xAD, 0x84, false}
#define g_cgb_42737b {0x42, 0x73, 0x7B, false}
#define g_cgb_ffff9c {0xFF, 0xFF, 0x9C, false}
#define g_cgb_94b5ff {0x94, 0xB5, 0xFF, false}
#define g_cgb_639473 {0x63, 0x94, 0x73, false}
#define g_cgb_003a3a {0x00, 0x3A, 0x3A, false}
#define g_cgb_6bff00 {0x6B, 0xFF, 0x00, false}
#define g_cgb_ff524a {0xFF, 0x52, 0x4A, false}
#define g_cgb_52de00 {0x52, 0xDE, 0x00, false}
#define g_cgb_ff8400 {0xFF, 0x84, 0x00, false}
#define g_cgb_ffff00 {0xFF, 0xFF, 0x00, false}
#define g_cgb_7bff00 {0x7B, 0xFF, 0x00, false}
#define g_cgb_b57300 {0xB5, 0x73, 0x00, false}
#define g_cgb_ffffce {0xFF, 0xFF, 0xCE, false}
#define g_cgb_63efef {0x63, 0xEF, 0xEF, false}
#define g_cgb_9c8431 {0x9C, 0x84, 0x31, false}
#define g_cgb_5a5a5a {0x5A, 0x5A, 0x5A, false}
#define g_cgb_b5b5ff {0xB5, 0xB5, 0xFF, false}
#define g_cgb_ffff94 {0xFF, 0xFF, 0x94, false}
#define g_cgb_ad5a42 {0xAD, 0x5A, 0x42, false}
#define g_cgb_63a5ff {0x63, 0xA5, 0xFF, false}
#define g_cgb_8c8cde {0x8C, 0x8C, 0xDE, false}
#define g_cgb_52528c {0x52, 0x52, 0x8C, false}
#define g_cgb_7bff31 {0x7B, 0xFF, 0x31, false}
#define g_cgb_008400 {0x00, 0x84, 0x00, false}
#define g_cgb_ffad63 {0xFF, 0xAD, 0x63, false}
#define g_cgb_843100 {0x84, 0x31, 0x00, false}
#define g_cgb_ff8484 {0xFF, 0x84, 0x84, false}
#define g_cgb_943a3a {0x94, 0x3A, 0x3A, false}
#define g_cgb_ffe6c5 {0xFF, 0xE6, 0xC5, false}
#define g_cgb_ce9c84 {0xCE, 0x9C, 0x84, false}
#define g_cgb_846b29 {0x84, 0x6b, 0x29, false}
#define g_cgb_5a3108 {0x5A, 0x31, 0x08, false}
#define g_cgb_7b4a00 {0x7B, 0x4A, 0x00, false}
#define g_cgb_0063c5 {0x00, 0x63, 0xC5, false}
#define g_cgb_ffc542 {0xFF, 0xC5, 0x42, false}
#define g_cgb_ffd600 {0xFF, 0xD6, 0x00, false}
#define g_cgb_943a00 {0x94, 0x3A, 0x00, false}
#define g_cgb_4a0000 {0x4A, 0x00, 0x00, false}
#define g_cgb_ff0000 {0xFF, 0x00, 0x00, false}
#define g_cgb_9c6300 {0x9C, 0x63, 0x00, false}
#define g_cgb_ffce00 {0xFF, 0xCE, 0x00, false}
#define g_cgb_006300 {0x00, 0x63, 0x00, false}
#define g_cgb_008484 {0x00, 0x84, 0x84, false}
#define g_cgb_ffde00 {0xFF, 0xDE, 0x00, false}
#define g_cgb_a5a5a5 {0xA5, 0xA5, 0xA5, false}
#define g_cgb_525252 {0x52, 0x52, 0x52, false}
#define g_cgb_52ff00 {0x52, 0xFF, 0x00, false}
#define g_cgb_ff4200 {0xFF, 0x42, 0x00, false}
#define g_cgb_ff9c00 {0xFF, 0x9C, 0x00, false}
#define g_cgb_a59cff {0xA5, 0x9C, 0xFF, false}
#define g_cgb_ff7300 {0xFF, 0x73, 0x00, false}
#define g_cgb_944200 {0x94, 0x42, 0x00, false}
#define g_cgb_ff6352 {0xFF, 0x63, 0x52, false}
#define g_cgb_d60000 {0xD6, 0x00, 0x00, false}
#define g_cgb_630000 {0x63, 0x00, 0x00, false}
#define g_cgb_00ff00 {0x00, 0xFF, 0x00, false}
#define g_cgb_318400 {0x31, 0x84, 0x00, false}
#define g_cgb_004a00 {0x00, 0x4a, 0x00, false}
#define g_cgb_ffffa5 {0xFF, 0xFF, 0xA5, false}
#define g_cgb_ff9494 {0xFF, 0x94, 0x94, false}
#define g_cgb_9494ff {0x94, 0x94, 0xFF, false}
#define g_cgb_5abdff {0x5A, 0xBD, 0xFF, false}
#define g_cgb_ffff7b {0xFF, 0xFF, 0x7B, false}
#define g_cgb_0084ff {0x00, 0x84, 0xFF, false}
#define g_cgb_555555 {0x55, 0x55, 0x55, false}
#define g_cgb_aaaaaa {0xAA, 0xAA, 0xAA, false}
#define g_cgb_ffffff {0xFF, 0xFF, 0xFF, false}


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


typedef struct palette_config {
	colour bg_palette[4];
	colour obj0_palette[4];
	colour obj1_palette[4];
} palette_config;


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
static s16       g_mode_clocks_counter             = 0;
static a16       g_window_tile_map_display_address = 0;
static a16       g_bg_window_tile_data_address     = 0;
static const a16 g_sprite_tile_data_address        = 0x8000;
static a16       g_bg_tile_map_display_address     = 0;


static d8 background_palette_memory[64];
static d8 sprite_palette_memory[64];
static palette_config g_current_palette_configuration = {
		{ g_cgb_000000, g_cgb_555555, g_cgb_aaaaaa, g_cgb_ffffff },
		{ g_cgb_000000, g_cgb_555555, g_cgb_aaaaaa, g_cgb_ffffff },
		{ g_cgb_000000, g_cgb_555555, g_cgb_aaaaaa, g_cgb_ffffff }
};


static palette_config const g_palette_configurations[6][29] = {
	{
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 }
		},
		{
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a },
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a },
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a }
		},
		{
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 },
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 },
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 }
		},
		{
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff },
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff },
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 }
		},
		{
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 },
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 },
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a },
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a },
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a }
		},
		{
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 },
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 },
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff },
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff },
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 },
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 },
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 },
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 },
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 }
		}
	},
	{
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 }
		},
		{
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 },
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a }
		},
		{
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff },
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 }
		},
		{
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff },
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 }
		},
		{
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 },
			{ g_cgb_ff6352, g_cgb_d60000, g_cgb_630000, g_cgb_000000 },
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 },
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a }
		},
		{
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 },
			{ g_cgb_000000, g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a },
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_00ff00, g_cgb_318400, g_cgb_004a00 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffff00, g_cgb_ff0000, g_cgb_630000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 }
		}
	},
	{
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 }
		},
		{
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a },
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 }
		},
		{
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 },
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff }
		},
		{
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff },
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 },
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 },
			{ g_cgb_ff6352, g_cgb_d60000, g_cgb_630000, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a },
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 }
		},
		{
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 },
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 },
			{ g_cgb_000000, g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_00ff00, g_cgb_318400, g_cgb_004a00 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff },
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffff00, g_cgb_ff0000, g_cgb_630000, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 },
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 },
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		}
	},
	{
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 }
		},
		{
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 }
		},
		{
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff }
		},
		{
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 },
			{ g_cgb_ff6352, g_cgb_d60000, g_cgb_630000, g_cgb_000000 },
			{ g_cgb_ff6352, g_cgb_d60000, g_cgb_630000, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 }
		},
		{
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 },
			{ g_cgb_000000, g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a },
			{ g_cgb_000000, g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_00ff00, g_cgb_318400, g_cgb_004a00 },
			{ g_cgb_ffffff, g_cgb_00ff00, g_cgb_318400, g_cgb_004a00 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffff00, g_cgb_ff0000, g_cgb_630000, g_cgb_000000 },
			{ g_cgb_ffff00, g_cgb_ff0000, g_cgb_630000, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		}
	},
	{
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a },
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a },
			{ g_cgb_ffc542, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 },
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff },
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 },
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 },
			{ g_cgb_0000ff, g_cgb_ffffff, g_cgb_ffff7b, g_cgb_0084ff }
		},
		{
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a },
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 },
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 },
			{ g_cgb_000000, g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff7b, g_cgb_0084ff, g_cgb_ff0000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff },
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffff00, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 },
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 },
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		}
	},
	{
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffff9c, g_cgb_94b5ff, g_cgb_639473, g_cgb_003a3a },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_6bff00, g_cgb_ffffff, g_cgb_ff524a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_52de00, g_cgb_ff8400, g_cgb_ffff00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff00, g_cgb_b57300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_52ff00, g_cgb_ff4200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff9c00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_ff0000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_a59cff, g_cgb_ffff00, g_cgb_006300, g_cgb_000000 },
			{ g_cgb_ff6352, g_cgb_d60000, g_cgb_630000, g_cgb_000000 },
			{ g_cgb_0000ff, g_cgb_ffffff, g_cgb_ffff7b, g_cgb_0084ff }
		},
		{
			{ g_cgb_ffffce, g_cgb_63efef, g_cgb_9c8431, g_cgb_5a5a5a },
			{ g_cgb_ffffff, g_cgb_ff7300, g_cgb_944200, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_b5b5ff, g_cgb_ffff94, g_cgb_ad5a42, g_cgb_000000 },
			{ g_cgb_000000, g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a },
			{ g_cgb_000000, g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffff7b, g_cgb_0084ff, g_cgb_ff0000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffc542, g_cgb_ffd600, g_cgb_943a00, g_cgb_4a0000 },
			{ g_cgb_ffffff, g_cgb_5abdff, g_cgb_ff0000, g_cgb_0000ff }
		},
		{
			{ g_cgb_ffffff, g_cgb_8c8cde, g_cgb_52528c, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_00ff00, g_cgb_318400, g_cgb_004a00 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_000000, g_cgb_008484, g_cgb_ffde00, g_cgb_ffffff },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffff00, g_cgb_ff0000, g_cgb_630000, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_adad84, g_cgb_42737b, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_a5a5a5, g_cgb_525252, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffa5, g_cgb_ff9494, g_cgb_9494ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffe6c5, g_cgb_ce9c84, g_cgb_846b29, g_cgb_5a3108 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffad63, g_cgb_843100, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffff00, g_cgb_7b4a00, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_008400, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ffce00, g_cgb_9c6300, g_cgb_000000 }
		},
		{
			{ g_cgb_ffffff, g_cgb_7bff31, g_cgb_0063c5, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_ff8484, g_cgb_943a3a, g_cgb_000000 },
			{ g_cgb_ffffff, g_cgb_63a5ff, g_cgb_0000ff, g_cgb_000000 }
		}
	}
};

static colour g_gpu_screen[SCREEN_HEIGHT][SCREEN_WIDTH];

#endif /* GPU_STATICS_H_ */
