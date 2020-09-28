#include<stdbool.h>
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
static u16       g_mode_2_boundary                 = 0;
static u16       g_mode_3_boundary                 = 0;
static a16       g_window_tile_map_display_address = 0;
static a16       g_bg_window_tile_data_address     = 0;
static const a16 g_sprite_tile_data_address        = 0x8000;
static a16       g_bg_tile_map_display_address     = 0;


//TODO: find why bg palette is not filled.
static d8 background_palette_memory[64] = {0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
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


static void _gpu_error(enum logger_log_type type, char *title, char *message)
{
	logger_log(
		type,
		title,
		"[GPU MODULE] %s\n",
		message
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
	u8 obp = 0;
	switch(palette_number) {
	case 0:
		obp = g_gpu_reg.obp0;
		break;
	case 1:
		obp = g_gpu_reg.obp1;
		break;
	}

	//Acquire colour value
	obp >>= colour_number * 2;
	obp &= (B0 | B1);

	//Translate colour value to a colour variable
	switch(palette_number) {
	case 0:
		found_colour = g_current_palette_configuration.obj0_palette[obp];
		break;
	case 1:
		found_colour = g_current_palette_configuration.obj1_palette[obp];
		break;
	}

	found_colour.a = (colour_number == 0) ? true : false;
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
	found_colour = g_current_palette_configuration.bg_palette[bgp];

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
	current_sprite.has_priority_over_bg_1_3 = (bit_data & B7) == 0;

	return current_sprite;
}


//vram_bank_number is 255-nullable
static void _gpu_get_colour_numbers(
	a16 base_address,
	s16 tile_number,
	u8 line_index,
	u8 vram_bank_number,
	bool flip_x,
	u8 dst[8]
)
{
	//Get line
	d8  line_upper, line_lower;
	a16 first_addr = base_address + tile_number * 2 * 8 + line_index * 2;
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
	//d8 tile_number;
	d8 line_index;
	for(u8 i = 0; i < sprite_index; i++)
	{
		//Check which line we are getting
		line_index = sprites[i].flipped_y
				? g_sprite_height - 1 - (ly - sprites[i].y)
				: (ly - sprites[i].y);

		//Get tile address
		/* TODO this part is incorrect please, look into it:
		if(g_sprite_height == 16)
			if(line_index > 8)
				tile_number = sprites[i].tile_number | B0;
			else
				tile_number = sprites[i].tile_number & !B0;
		else
			tile_number = sprites[i].tile_number;
		*/

		//Get single sprite colour numbers
		_gpu_get_colour_numbers(
			g_sprite_tile_data_address,
			sprites[i].tile_number,
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
				|| bg_colour_is_0[current_index]
				|| (
					!bg_bit_7[current_index]
					&& !bg_colour_is_0[current_index]
					&& sprites[i].has_priority_over_bg_1_3
				)
			) {
				if(current_index >= 160)
					break;
				colour col = _gpu_get_colour(
						colour_numbers[i][j],
						rom_is_cgb() ? sprites[i].palette_number_cgb : sprites[i].palette_number_gb,
						SPRITE);
				if(!col.a)
					line[current_index] = col;
			}
		}
	}
}


static void _gpu_get_tile_number_attr(
	a16 map_address,
	u16 tile_map_number,
	a16 data_address,
	s16* tile_number,
	bg_attr* tile_attr
)
{
	u8  u_tile_number;
	s8  s_tile_number;
	d8 tile_attr_byte;

	u_tile_number = (u8) mem_vram_read8(0, map_address + tile_map_number);
	s_tile_number = (s8) u_tile_number;
	*tile_number  = (s16) ((data_address == 0x9000) ? s_tile_number : u_tile_number);

	if(rom_is_cgb()) {
		tile_attr_byte = mem_vram_read8(1, map_address + tile_map_number);
		tile_attr->bgp_number            =  tile_attr_byte & (B2 | B1 | B0);
		tile_attr->vram_bank_number      = (tile_attr_byte & (B3)) >> 3;
		tile_attr->flipped_x             = (tile_attr_byte & (B5)) == B5;
		tile_attr->flipped_y             = (tile_attr_byte & (B6)) == B6;
		tile_attr->has_priority_over_oam = (tile_attr_byte & (B7)) == B7;
	}
}


static void _gpu_put_window(colour line[160], bool bg_bit_7[160], bool bg_colour_is_0[160])
{
	//Get data
	u8  ly              = g_gpu_reg.ly;
	u8  wy              = g_gpu_reg.wy;
	s16 wx              = g_gpu_reg.wx - 7;
	s16 tile_map_y      = ly - wy;
	u8  tile_map_x      = (wx < 0) ? 7 : wx;
	u8  tile_map_tile_y = (tile_map_y - tile_map_y % 8) / 8;
	u8  tile_map_tile_x = (tile_map_x - tile_map_x % 8) / 8;

	//Is the window on screen right now?
	if((ly < wy) || (wy > 143) || (wx > 159)) {
		return;
	}

	//Get proper tile from tile map
	s16 tile_map_index;
	s16 tile_number;
	u8  tile_colour_numbers[8];
	bg_attr tile_attr = {0};
	for(u8 i = 0; i < 20; i++)
	{
		//What's our tile map number (index)
		tile_map_index = tile_map_tile_x + i + tile_map_tile_y * 32;

		//Get tile number and attributes, if able
		_gpu_get_tile_number_attr(
			g_window_tile_map_display_address,
			tile_map_index,
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
			current_index = (wx + i * 8 + j) % 160;
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
	u8 ly              = g_gpu_reg.ly;
	u8 scy             = g_gpu_reg.scy;
	u8 scx             = g_gpu_reg.scx;
	u8 tile_map_y      = (scy + ly) % 256;
	u8 tile_map_x      = scx;
	u8 tile_map_tile_y = tile_map_y / 8;
	u8 tile_map_tile_x = tile_map_x / 8;

	//Get proper tile from tile map
	s16 tile_map_index;
	s16 tile_number;
	u8  tile_colour_numbers[8];
	bg_attr tile_attr = {0};
	// 21 because we have to account for x scrolling
	u8 current_index = 0;
	for(u8 i = 0; i < 21; i++)
	{
		//What's our tile map index
		tile_map_index = (tile_map_tile_x + i)%32 + tile_map_tile_y * 32;

		//Get tile number and attributes, if able
		_gpu_get_tile_number_attr(
			g_bg_tile_map_display_address,
			tile_map_index,
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
		u8 offset;
		u8 j = 0;
		offset = i == 0 ? scx % 8 : 0;
		for(j = offset; j < 8; j++)
		{
			if(current_index >= 160)
				break;
			bg_colour_is_0[current_index] = tile_colour_numbers[j] == 0;
			bg_bit_7[current_index]       = rom_is_cgb() ? tile_attr.has_priority_over_oam : false;
			line[current_index] = _gpu_get_colour(
				tile_colour_numbers[j],
				rom_is_cgb() ? tile_attr.bgp_number : 0,
				BACKGROUND
			);
			current_index++;
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
				line[i]           = (colour)g_cgb_ffffff;
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


static u8 _gpu_compute_hash(char * title_buffer)
{
	//The 8-bit sum of the title, which is only upper-case ASCII and spaces
	u8 hash = 0;

	//Title length is annoying - it can be between 11 and 15 characters
	for(int i=0; i<11; i++)
		hash += title_buffer[i];

	for(int i=11; i<15; i++)
	{
		if(title_buffer[i] == 0)
			break;
		if(
			title_buffer[i] == ' '
			|| (title_buffer[i]>='A' || title_buffer[i]<='Z')
		)
			hash += title_buffer[i];
	}

	return hash;
}


static void _gpu_check_assigned_palette_configurations(void)
{
	if(!rom_is_licensee()) {
		g_current_palette_configuration = g_palette_configurations[3][28];
		return;
	}

	char title[16];
	rom_get_title(title);

	switch(_gpu_compute_hash(title) ) {
	case 0x71:
	case 0xFF:
		g_current_palette_configuration = g_palette_configurations[0][0x06];
		break;
	case 0x15:
	case 0xDB:
		g_current_palette_configuration = g_palette_configurations[0][0x07];
		break;
	case 0x88:
		g_current_palette_configuration = g_palette_configurations[0][0x08];
		break;
	case 0x0C:
	case 0x16:
	case 0x35:
	case 0x67:
	case 0x75:
	case 0x92:
	case 0x99:
	case 0xB7:
		g_current_palette_configuration = g_palette_configurations[0][0x12];
		break;
	case 0x28:
		switch(title[3]) {
		case 0x41:
			g_current_palette_configuration = g_palette_configurations[0][0x13];
			break;
		case 0x46:
			g_current_palette_configuration = g_palette_configurations[3][0x0E];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0xA5:
		switch(title[3]) {
		case 0x41:
			g_current_palette_configuration = g_palette_configurations[0][0x13];
			break;
		case 0x52:
			g_current_palette_configuration = g_palette_configurations[3][0x12];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0xE8:
		g_current_palette_configuration = g_palette_configurations[0][0x08];
		break;
	case 0x58:
		g_current_palette_configuration = g_palette_configurations[0][0x16];
		break;
	case 0x6F:
		g_current_palette_configuration = g_palette_configurations[0][0x1B];
		break;

	case 0x8C:
		g_current_palette_configuration = g_palette_configurations[1][0x00];
		break;
	case 0x61:
		switch(title[3]) {
		case 0x41:
			g_current_palette_configuration = g_palette_configurations[5][0x0E];
			break;
		case 0x45:
			g_current_palette_configuration = g_palette_configurations[1][0x0B];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0xD3:
		switch(title[3]) {
		case 0x49:
			g_current_palette_configuration = g_palette_configurations[5][0x15];
			break;
		case 0x52:
			g_current_palette_configuration = g_palette_configurations[1][0x0D];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0x14:
		g_current_palette_configuration = g_palette_configurations[1][0x10];
		break;
	case 0xAA:
		g_current_palette_configuration = g_palette_configurations[1][0x1C];
		break;

	case 0x3C:
		g_current_palette_configuration = g_palette_configurations[2][0x0B];
		break;
	case 0x9C:
		g_current_palette_configuration = g_palette_configurations[2][0x0C];
		break;

	case 0xB3:
		switch(title[3]) {
		case 0x42:
			g_current_palette_configuration = g_palette_configurations[5][0x08];
			break;
		case 0x52:
			g_current_palette_configuration = g_palette_configurations[4][0x05];
			break;
		case 0x55:
			g_current_palette_configuration = g_palette_configurations[3][0x00];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0x66:
		switch(title[3]) {
		case 0x45:
			g_current_palette_configuration = g_palette_configurations[3][0x04];
			break;
		case 0x4C:
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0xF4:
		switch(title[3]) {
		case 0x20:
			g_current_palette_configuration = g_palette_configurations[3][0x04];
			break;
		case 0x2D:
			g_current_palette_configuration = g_palette_configurations[5][0x1C];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0x34:
		g_current_palette_configuration = g_palette_configurations[3][0x04];
		break;
	case 0x6A:
		switch(title[3]) {
		case 0x49:
			g_current_palette_configuration = g_palette_configurations[3][0x05];
			break;
		case 0x4B:
			g_current_palette_configuration = g_palette_configurations[5][0x0C];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0x3D:
		g_current_palette_configuration = g_palette_configurations[3][0x05];
		break;
	case 0x19:
		g_current_palette_configuration = g_palette_configurations[3][0x06];
		break;
	case 0x1D:
		g_current_palette_configuration = g_palette_configurations[3][0x08];
		break;
	case 0x46:
		switch(title[3]) {
		case 0x45:
			g_current_palette_configuration = g_palette_configurations[3][0x0A];
			break;
		case 0x52:
			g_current_palette_configuration = g_palette_configurations[5][0x14];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0x0D:
		switch(title[3]) {
		case 0x45:
			g_current_palette_configuration = g_palette_configurations[3][0x0C];
			break;
		case 0x52:
			g_current_palette_configuration = g_palette_configurations[4][0x07];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0xBF:
		switch(title[3]) {
		case 0x20:
			g_current_palette_configuration = g_palette_configurations[3][0x0D];
			break;
		case 0x43:
			g_current_palette_configuration = g_palette_configurations[5][0x02];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0x4B:
	case 0x90:
	case 0x9A:
	case 0xBD:
		g_current_palette_configuration = g_palette_configurations[3][0x0E];
		break;
	case 0x39:
	case 0x43:
	case 0x97:
		g_current_palette_configuration = g_palette_configurations[3][0x0F];
		break;
	case 0x18:
		switch(title[3]) {
		case 0x49:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		case 0x4B:
			g_current_palette_configuration = g_palette_configurations[5][0x0C];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0xC6:
		switch(title[3]) {
		case 0x20:
			g_current_palette_configuration = g_palette_configurations[3][0x0E];
			break;
		case 0x41:
			g_current_palette_configuration = g_palette_configurations[5][0x00];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;

	case 0x95:
		g_current_palette_configuration = g_palette_configurations[4][0x0F];
		break;
	case 0x3E:
	case 0xE0:
		g_current_palette_configuration = g_palette_configurations[4][0x06];
		break;
	case 0x69:
	case 0xF2:
		g_current_palette_configuration = g_palette_configurations[4][0x07];
		break;

	case 0x59:
		g_current_palette_configuration = g_palette_configurations[5][0x00];
		break;
	case 0x86:
	case 0xA8:
		g_current_palette_configuration = g_palette_configurations[5][0x01];
		break;
	case 0xCE:
	case 0xD1:
	case 0xF0:
		g_current_palette_configuration = g_palette_configurations[5][0x02];
		break;
	case 0x36:
		g_current_palette_configuration = g_palette_configurations[5][0x03];
		break;
	case 0x49:
	case 0x5C:
		g_current_palette_configuration = g_palette_configurations[5][0x08];
		break;
	case 0x27:
		switch(title[3]) {
		case 0x42:
			g_current_palette_configuration = g_palette_configurations[5][0x08];
			break;
		case 0x4E:
			g_current_palette_configuration = g_palette_configurations[5][0x0E];
			break;
		default:
			g_current_palette_configuration = g_palette_configurations[3][0x1C];
			break;
		}
		break;
	case 0xC9:
		g_current_palette_configuration = g_palette_configurations[5][0x09];
		break;
	case 0x4E:
		g_current_palette_configuration = g_palette_configurations[5][0x0B];
		break;
	case 0x6B:
		g_current_palette_configuration = g_palette_configurations[5][0x0C];
		break;
	case 0x9D:
		g_current_palette_configuration = g_palette_configurations[5][0x0D];
		break;
	case 0x17:
	case 0x8B:
		g_current_palette_configuration = g_palette_configurations[5][0x0E];
		break;
	case 0x01:
	case 0x10:
	case 0x29:
	case 0x52:
	case 0x5D:
	case 0x68:
	case 0x6D:
	case 0xF6:
		g_current_palette_configuration = g_palette_configurations[5][0x0F];
		break;
	case 0x70:
		g_current_palette_configuration = g_palette_configurations[5][0x11];
		break;
	case 0xA2:
	case 0xF7:
		g_current_palette_configuration = g_palette_configurations[5][0x12];
		break;

	default:
		g_current_palette_configuration = g_palette_configurations[3][0x1C];
		break;
	}
}


static u8 _gpu_read_bgpi(void)
{
	return g_gpu_reg.bgpi;
}


static void _gpu_write_bgpi(u8 new_bgpi)
{
	g_gpu_reg.bgpi = new_bgpi;
}


static u8 _gpu_read_bgpd(void)
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


static void _gpu_write_bgpd(u8 new_bgpd)
{
	//Only accessible during H-BLANK or V-BLANK
	u8 stat = g_gpu_reg.stat;
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK) {
		g_gpu_reg.bgpd = new_bgpd;

		//Update BGP
		u8 bgpi = _gpu_read_bgpi();
		_gpu_write_bgpm((bgpi & 0x1F), new_bgpd);

		//Increment BGPI if required
		if((bgpi & B7) == B7) {
			d8 new_bgpi = bgpi;
			new_bgpi++;
			new_bgpi &= !B6;
			_gpu_write_bgpi(new_bgpi);
		}
	} else {
		_gpu_error(
			LOG_FATAL,
			"GPU SPD",
			"SPD REGISTER ACCESSED OUTSIDE H- OR V-BLANK"
		);
	}
}


static u8 _gpu_read_spi(void)
{
	return g_gpu_reg.spi;
}


static void _gpu_write_spi(u8 new_spi)
{
	g_gpu_reg.spi = new_spi;
}


static u8 _gpu_read_spd(void)
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


static void _gpu_write_spd(u8 new_spd)
{
	//Only accessible during H-BLANK or V-BLANK
	u8 stat = g_gpu_reg.stat;
	stat &= 0x03;
	if(stat == GPU_H_BLANK || stat == GPU_V_BLANK) {
		g_gpu_reg.spd = new_spd;

		//Update SP
		u8 spi = _gpu_read_spi();
		_gpu_write_spm((spi & 0x1F), new_spd);

		//Increment BGPI if required
		if((spi & B7) == B7) {
			u8 new_spi = spi;
			new_spi++;
			new_spi &= !B6;
			_gpu_write_spi(new_spi);
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
			return _gpu_read_bgpi();
			break;
		case BGPDAddress:
			return _gpu_read_bgpd();
			break;
		case SPIAddress:
			return _gpu_read_spi();
			break;
		case SPDAddress:
			return _gpu_read_spd();
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
			g_gpu_reg.scy  = data;
			break;
		case SCXAddress:
			g_gpu_reg.scx  = data;
			break;
		case LYAddress:
			g_gpu_reg.ly   = data;
			break;
		case LYCAddress:
			g_gpu_reg.lyc  = data;
			break;
		case BGPAddress:
			g_gpu_reg.bgp  = data;
			break;
		case OBP0Address:
			g_gpu_reg.obp0 = data;
			break;
		case OBP1Address:
			g_gpu_reg.obp1 = data;
			break;
		case WYAddress:
			g_gpu_reg.wy   = data;
			break;
		case WXAddress:
			g_gpu_reg.wx   = data;
			break;
		case BGPIAddress:
			_gpu_write_bgpi(data);
			g_gpu_reg.bgpd = _gpu_read_bgpd();
			break;
		case BGPDAddress:
			_gpu_write_bgpd(data);
			break;
		case SPIAddress:
			_gpu_write_spi(data);
			g_gpu_reg.spd = _gpu_read_spd();
			break;
		case SPDAddress:
			_gpu_write_spd(data);
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

	_gpu_check_assigned_palette_configurations();

	_gpu_restart_boundaries();

	display_prepare(1.0 / FRAME_RATE, rom_title);

	g_gpu_reg.lcdc = 0x91;
	g_gpu_reg.stat = 0x85;

	g_gpu_reg.obp0 = BGPDefault;
	g_gpu_reg.obp1 = BGPDefault;
	g_gpu_reg.bgp = BGPDefault;
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

		if(g_gpu_reg.ly < 144)
			_gpu_draw_scanline();

		//Increment the LY register
		g_gpu_reg.ly++;
	}
}


void gpu_destroy(void)
{
	display_destroy();
}
