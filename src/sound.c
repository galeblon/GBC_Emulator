#include"debug.h"
#include"events.h"
#include"ints.h"
#include"sound.h"
#include"mem_priv.h"
#include"types.h"

#define CH1_SWEEP_REG_ADDRESS 0xFF10
#define CH1_SND_LEN_PATTERN_ADDRESS 0xFF11
#define CH1_VOL_ENVELOPE_ADDRESS 0xFF12
#define CH1_FREQ_LO_ADDRESS 0xFF13
#define CH1_FREQ_HI_ADDRESS 0xFF14

#define CH2_SND_LEN_PATTERN_ADDRESS 0xFF16
#define CH2_VOL_ENVELOPE_ADDRESS 0xFF17
#define CH2_FREQ_LO_ADDRESS 0xFF18
#define CH2_FREQ_HI_ADDRESS 0xFF19

#define CH3_SND_ON_OFF_ADDRESS 0xFF1A
#define CH3_SND_LEN_ADDRESS 0xFF1B
#define CH3_SND_SEL_OUT_LVL_ADDRESS 0xFF1C
#define CH3_FREQ_LO_ADDRESS 0xFF1D
#define CH3_FREQ_HI_ADDRESS 0xFF1E

#define CH4_SND_LEN_ADDRESS 0xFF20
#define CH4_VOL_ENVELOPE_ADDRESS 0xFF21
#define CH4_POLY_COUNT_ADDRESS 0xFF22
#define CH4_COUNTER_CONSECUTIVE_ADDRESS 0xFF23

#define CHANNEL_CONTROL_ADDRESS 0xFF24
#define SOUND_TERMINAL_SELECTION_ADDRESS 0xFF25
#define SOUND_ON_OFF_ADDRESS 0xFF26

#define WAVEFORM_RAM_START_ADDRESS 0xFF30
#define WAVEFORM_RAM_END_ADDRESS 0xFF3F


// Channel 1
d8 g_channel1_sweep_register = 0;
d8 g_channel1_sound_length_wave_pattern_duty = 0;
d8 g_channel1_volume_envelope = 0;
d8 g_channel1_frequency_lo = 0;
d8 g_channel1_frequency_hi = 0;

// Channel 2
d8 g_channel2_sound_length_wave_pattern_duty = 0;
d8 g_channel2_volume_envelope = 0;
d8 g_channel2_frequency_lo = 0;
d8 g_channel2_frequency_hi = 0;

// Channel 3
d8 g_channel3_sound_on_off = 0;
d8 g_channel3_sound_length = 0;
d8 g_channel3_select_output_level = 0;
d8 g_channel3_frequency_lo = 0;
d8 g_channel3_frequency_hi = 0;
d8 g_wave_pattern_ram[16];

// Channel 4
d8 g_channel4_sound_length = 0;
d8 g_channel4_volume_envelope = 0;
d8 g_channel4_polynomial_counter = 0;
d8 g_channel4_counter_consecutive = 0;
d8 g_channel4_frequency_lo = 0;
d8 g_channel4_frequency_hi = 0;

// General
d8 g_channel_control = 0;
d8 g_sound_output_terminal = 0;
d8 g_sound_on_off;


static u8 _sound_read_handler(a16 addr)
{
	if(addr >= WAVEFORM_RAM_START_ADDRESS && addr <= WAVEFORM_RAM_END_ADDRESS) {
			return g_wave_pattern_ram[addr - WAVEFORM_RAM_START_ADDRESS];
	}
	switch(addr) {
		case CH1_SWEEP_REG_ADDRESS:
			return g_channel1_sweep_register;
			break;
		case CH1_SND_LEN_PATTERN_ADDRESS:
			return g_channel1_sound_length_wave_pattern_duty & 0xC0;
		case CH1_VOL_ENVELOPE_ADDRESS:
			return g_channel1_volume_envelope;
		case CH1_FREQ_LO_ADDRESS:
			debug_assert(false, "_sound_read_handler: write only address");
			return 0;
		case CH1_FREQ_HI_ADDRESS:
			return g_channel1_frequency_hi & 0x40;
		case CH2_SND_LEN_PATTERN_ADDRESS:
			return g_channel2_sound_length_wave_pattern_duty & 0xC0;
		case CH2_VOL_ENVELOPE_ADDRESS:
			return g_channel1_volume_envelope;
		case CH2_FREQ_LO_ADDRESS:
			debug_assert(false, "_sound_read_handler: write only address");
			return 0;
		case CH2_FREQ_HI_ADDRESS:
			return g_channel2_frequency_hi & 0x40;
		case CH3_SND_ON_OFF_ADDRESS:
			return g_channel3_sound_on_off;
		case CH3_SND_LEN_ADDRESS:
			return g_channel3_sound_length;
		case CH3_SND_SEL_OUT_LVL_ADDRESS:
			return g_channel3_select_output_level;
		case CH3_FREQ_LO_ADDRESS:
			debug_assert(false, "_sound_read_handler: write only address");
			return 0;
		case CH3_FREQ_HI_ADDRESS:
			return g_channel3_frequency_hi & 0x40;
		case CH4_SND_LEN_ADDRESS:
			return g_channel4_sound_length;
		case CH4_VOL_ENVELOPE_ADDRESS:
			return g_channel4_volume_envelope;
		case CH4_POLY_COUNT_ADDRESS:
			return g_channel4_polynomial_counter;
		case CH4_COUNTER_CONSECUTIVE_ADDRESS:
			return g_channel4_counter_consecutive & 0x7F;
		case CHANNEL_CONTROL_ADDRESS:
			return g_channel_control;
		case SOUND_TERMINAL_SELECTION_ADDRESS:
			return g_sound_output_terminal;
		case SOUND_ON_OFF_ADDRESS:
			return g_sound_on_off & (g_channel3_sound_on_off >> 4);
		default:
			debug_assert(false, "_sound_read_handler: invalid address");
			return 0;
	}
}

static void _sound_write_handler(a16 addr, u8 data)
{
	if(addr >= WAVEFORM_RAM_START_ADDRESS && addr <= WAVEFORM_RAM_END_ADDRESS) {
		g_wave_pattern_ram[addr - WAVEFORM_RAM_START_ADDRESS] = data;
		return;
	}
	switch(addr) {
		case CH1_SWEEP_REG_ADDRESS:
			g_channel1_sweep_register = data;
			break;
		case CH1_SND_LEN_PATTERN_ADDRESS:
			g_channel1_sound_length_wave_pattern_duty = data;
			break;
		case CH1_VOL_ENVELOPE_ADDRESS:
			g_channel1_volume_envelope = data;
			break;
		case CH1_FREQ_LO_ADDRESS:
			g_channel1_frequency_lo = data;
			break;
		case CH1_FREQ_HI_ADDRESS:
			g_channel1_frequency_hi = data;
			break;
		case CH2_SND_LEN_PATTERN_ADDRESS:
			g_channel2_sound_length_wave_pattern_duty = data;
			break;
		case CH2_VOL_ENVELOPE_ADDRESS:
			g_channel2_volume_envelope = data;
			break;
		case CH2_FREQ_LO_ADDRESS:
			g_channel2_frequency_lo = data;
			break;
		case CH2_FREQ_HI_ADDRESS:
			g_channel2_frequency_hi = data;
			break;
		case CH3_SND_ON_OFF_ADDRESS:
			g_channel3_sound_on_off = data & 0x80;
			break;
		case CH3_SND_LEN_ADDRESS:
			g_channel3_sound_length = data;
			break;
		case CH3_SND_SEL_OUT_LVL_ADDRESS:
			g_channel3_select_output_level = data & 0x60;
			break;
		case CH3_FREQ_LO_ADDRESS:
			g_channel3_frequency_lo = data;
			break;
		case CH3_FREQ_HI_ADDRESS:
			g_channel3_frequency_hi = data;
			break;
		case CH4_SND_LEN_ADDRESS:
			g_channel4_sound_length = data & 0x3F;
			break;
		case CH4_VOL_ENVELOPE_ADDRESS:
			g_channel4_volume_envelope = data;
			break;
		case CH4_POLY_COUNT_ADDRESS:
			g_channel4_polynomial_counter = data;
			break;
		case CH4_COUNTER_CONSECUTIVE_ADDRESS:
			g_channel4_counter_consecutive = data;
			break;
		case CHANNEL_CONTROL_ADDRESS:
			g_channel_control = data;
			break;
		case SOUND_TERMINAL_SELECTION_ADDRESS:
			g_sound_output_terminal = data;
			break;
		case SOUND_ON_OFF_ADDRESS:
			g_sound_on_off &= 0x7F;
			g_sound_on_off |= data & 0x80;
			break;
		default:
			debug_assert(false, "_sound_write_handler: invalid address");
		}
}

void sound_prepare(void)
{
	for (a16 addr = CH1_SWEEP_REG_ADDRESS; addr <= CH1_FREQ_HI_ADDRESS; addr++) {
		mem_register_handlers(addr,
				_sound_read_handler, _sound_write_handler);
	}

	for (a16 addr = CH2_SND_LEN_PATTERN_ADDRESS; addr <= CH3_FREQ_HI_ADDRESS; addr++) {
		mem_register_handlers(addr,
				_sound_read_handler, _sound_write_handler);
	}

	for (a16 addr = CH4_SND_LEN_ADDRESS; addr <= SOUND_ON_OFF_ADDRESS; addr++) {
		mem_register_handlers(addr,
				_sound_read_handler, _sound_write_handler);
	}

	for (a16 addr = WAVEFORM_RAM_START_ADDRESS; addr <= WAVEFORM_RAM_END_ADDRESS; addr++) {
		mem_register_handlers(addr,
				_sound_read_handler, _sound_write_handler);
	}
}
