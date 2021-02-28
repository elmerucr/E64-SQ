/*
 * common.hpp
 * E64-II
 *
 * Copyright © 2017-2021 elmerucr. All rights reserved.
 *
 * General definitions for the project
 */
 
#ifndef COMMON_H
#define COMMON_H

#include <cstdint>

#include "host.hpp"
#include "machine.hpp"
#include "stats.hpp"
#include "rom.hpp"

#define E64_SQ_MAJOR_VERSION    0
#define E64_SQ_MINOR_VERSION    1
#define E64_SQ_BUILD            20210227
#define E64_SQ_YEAR             2021

/* Global objects */
extern E64::host_t	host;
extern E64::machine_t	machine;
extern E64::stats_t	stats;

#define	RAM_SIZE	0x01000000   // 16mb system, DO NOT CHANGE

/*
 * System uses standard 720p60 HDMI output.
 * In a 'distant' future, this will come in handy with real hardware.
 * See also:
 * https://timetoexplore.net/blog/video-timings-vga-720p-1080p
 *
 * Name         1280x720p60
 * Standard       CTA-770.3
 * VIC                    4
 * Short Name          720p
 * Aspect Ratio        16:9
 *
 * Pixel Clock       74.250 MHz
 * TMDS Clock       742.500 MHz
 * Pixel Time          13.5 ns ±0.5%
 * Horizontal Freq.  45.000 kHz
 * Line Time           22.2 μs
 * Vertical Freq.    60.000 Hz
 * Frame Time          16.7 ms
 *
 * Horizontal Timings
 * Active Pixels       1280
 * Front Porch          110
 * Sync Width            40
 * Back Porch           220
 * Blanking Total       370
 * Total Pixels        1650
 * Sync Polarity        pos
 *
 * Vertical Timings
 * Active Lines         720
 * Front Porch            5
 * Sync Width             5
 * Back Porch            20
 * Blanking Total        30
 * Total Lines          750
 * Sync Polarity        pos
 *
 * Active Pixels    921,600
 * Data Rate           1.78 Gbps
 *
 * Frame Memory (Kbits)
 *  8-bit Memory      7,200
 * 12-bit Memory     10,800
 * 24-bit Memory     21,600
 * 32-bit Memory     28,800
 */
#define VICV_PIXELS_PER_SCANLINE	512
#define VICV_PIXELS_HBLANK		148
#define VICV_SCANLINES			288
#define VICV_SCANLINES_VBLANK		12
#define VICV_FB0			0x00f00000
#define VICV_FB1			0x00f48000

/* These two macros are defined for use within the monitor */
#define VICV_CHAR_COLUMNS		(VICV_PIXELS_PER_SCANLINE/8)
#define VICV_CHAR_ROWS			(VICV_SCANLINES/8)

#define FPS				60

#define VICV_DOT_CLOCK_SPEED        (VICV_PIXELS_PER_SCANLINE+VICV_PIXELS_HBLANK)*(VICV_SCANLINES+VICV_SCANLINES_VBLANK)*FPS
#define BLITTER_DOT_CLOCK_SPEED     (4*VICV_DOT_CLOCK_SPEED)

#define SID_CLOCK_SPEED             985248
#define SAMPLE_RATE                 44100
#define AUDIO_BUFFER_SIZE           8192.0

/* C64 colors (VirtualC64) */
#define C64_BLACK       0xf000
#define C64_WHITE       0xffff
#define C64_RED         0xf733
#define C64_CYAN        0xf8cc
#define C64_PURPLE      0xf849
#define C64_GREEN       0xf6a5
#define C64_BLUE        0xf339
#define C64_YELLOW      0xfee8
#define C64_ORANGE      0xf853
#define C64_BROWN       0xf531
#define C64_LIGHTRED    0xfb77
#define C64_DARKGREY    0xf444
#define C64_GREY        0xf777
#define C64_LIGHTGREEN  0xfbfa
#define C64_LIGHTBLUE   0xf67d
#define C64_LIGHTGREY   0xfaaa

/* Grey */
#define GREY_00 0xf000
#define GREY_01 0xf111
#define GREY_02 0xf222
#define GREY_03 0xf333
#define GREY_04 0xf444
#define GREY_05 0xf555
#define GREY_06 0xf666
#define GREY_07 0xf777
#define GREY_08 0xf888
#define GREY_09 0xf999
#define GREY_10 0xfaaa
#define GREY_11 0xfbbb
#define GREY_12 0xfccc
#define GREY_13 0xfddd
#define GREY_14 0xfeee
#define GREY_15 0xffff

/* Green */
#define GREEN_00  0xf000
#define GREEN_01  0xf121
#define GREEN_02  0xf242
#define GREEN_03  0xf363
#define GREEN_04  0xf484
#define GREEN_05  0xf5a5
#define GREEN_06  0xf6c6
#define GREEN_07  0xf7e7

/* Cobalt */
#define COBALT_00   0xf000
#define COBALT_01   0xf112
#define COBALT_02   0xf224
#define COBALT_03   0xf336
#define COBALT_04   0xf448
#define COBALT_05   0xf55a
#define COBALT_06   0xf66c
#define COBALT_07   0xf77e

/* Amber */
#define AMBER_00   0xf000
#define AMBER_01   0xf211
#define AMBER_02   0xf422
#define AMBER_03   0xf633
#define AMBER_04   0xf844
#define AMBER_05   0xfa55
#define AMBER_06   0xfc66
#define AMBER_07   0xfe77

/* Ascii values (some of them are petscii) */
#define ASCII_NULL          0x00    // null
#define ASCII_BACKSPACE     0x08
#define ASCII_HOR_TAB       0x09
#define ASCII_CR            0x0d    // carriage return
#define ASCII_LF            0x0a    // linefeed
#define ASCII_CURSOR_DOWN   0x11    // petscii
#define ASCII_REVERSE_ON    0x12    // petscii
#define ASCII_ESCAPE        0x1b
#define ASCII_CURSOR_RIGHT  0x1d    // petscii
#define ASCII_SPACE         0x20    // space
#define ASCII_EXCL_MARK     0x21    // !
#define ASCII_DOUBLE_QUOTES 0x22    // "
#define ASCII_NUMBER        0x23    // #
#define ASCII_DOLLAR        0x24    // $
#define ASCII_PERCENT       0x25    // %
#define ASCII_AMPERSAND     0x26    // &
#define ASCII_SINGLE_QUOTE  0x27    // '
#define ASCII_OPEN_PAR      0x28    // (
#define ASCII_CLOSE_PAR     0x29    // )
#define ASCII_ASTERISK      0x2a    // *
#define ASCII_PLUS          0x2b    // +
#define ASCII_COMMA         0x2c    // ,
#define ASCII_HYPHEN        0x2d    // -
#define ASCII_PERIOD        0x2e    // .
#define ASCII_SLASH         0x2f    // /
#define ASCII_0             0x30    // 0
#define ASCII_1             0x31    // 1
#define ASCII_2             0x32    // 2
#define ASCII_3             0x33    // 3
#define ASCII_4             0x34    // 4
#define ASCII_5             0x35    // 5
#define ASCII_6             0x36    // 6
#define ASCII_7             0x37    // 7
#define ASCII_8             0x38    // 8
#define ASCII_9             0x39    // 9
#define ASCII_COLON         0x3a    // :
#define ASCII_SEMI_COLON    0x3b    // ;
#define ASCII_LESS          0x3c    // <
#define ASCII_EQUALS        0x3d    // =
#define ASCII_GREATER       0x3e    // >
#define ASCII_QUESTION_M    0x3f    // ?
#define ASCII_AT            0x40    // @
#define ASCII_A             0x41
#define ASCII_B             0x42
#define ASCII_C             0x43
#define ASCII_D             0x44
#define ASCII_E             0x45
#define ASCII_F             0x46
#define ASCII_G             0x47
#define ASCII_H             0x48
#define ASCII_I             0x49
#define ASCII_J             0x4a
#define ASCII_K             0x4b
#define ASCII_L             0x4c
#define ASCII_M             0x4d
#define ASCII_N             0x4e
#define ASCII_O             0x4f
#define ASCII_P             0x50
#define ASCII_Q             0x51
#define ASCII_R             0x52
#define ASCII_S             0x53
#define ASCII_T             0x54
#define ASCII_U             0x55
#define ASCII_V             0x56
#define ASCII_W             0x57
#define ASCII_X             0x58
#define ASCII_Y             0x59
#define ASCII_Z             0x5a
#define ASCII_OPEN_BRACK    0x5b    // [
#define ASCII_BACKSLASH     0x5c    // \    patched
#define ASCII_CLOSE_BRACK   0x5d    // ]
#define ASCII_CARET         0x5e    // ^    patched
#define ASCII_UNDERSCORE    0x5f    // _
#define ASCII_GRAVE         0x60    // `
#define ASCII_a             0x61
#define ASCII_b             0x62
#define ASCII_c             0x63
#define ASCII_d             0x64
#define ASCII_e             0x65
#define ASCII_f             0x66
#define ASCII_g             0x67
#define ASCII_h             0x68
#define ASCII_i             0x69
#define ASCII_j             0x6a
#define ASCII_k             0x6b
#define ASCII_l             0x6c
#define ASCII_m             0x6d
#define ASCII_n             0x6e
#define ASCII_o             0x6f
#define ASCII_p             0x70
#define ASCII_q             0x71
#define ASCII_r             0x72
#define ASCII_s             0x73
#define ASCII_t             0x74
#define ASCII_u             0x75
#define ASCII_v             0x76
#define ASCII_w             0x77
#define ASCII_x             0x78
#define ASCII_y             0x79
#define ASCII_z             0x7a
#define ASCII_OPEN_BRACE    0x7b    // {
#define ASCII_VERT_BAR      0x7c    // |
#define ASCII_CLOSE_BRACE   0x7d    // }
#define ASCII_TILDE         0x7e    // ~
#define ASCII_DELETE        0x7f

#define ASCII_F1            0x85    // taken from cbm petscii
#define ASCII_F2            0x86    // taken from cbm petscii
#define ASCII_F3            0x87    // taken from cbm petscii
#define ASCII_F4            0x88    // taken from cbm petscii
#define ASCII_F5            0x89    // taken from cbm petscii
#define ASCII_F6            0x8a    // taken from cbm petscii
#define ASCII_F7            0x8b    // taken from cbm petscii
#define ASCII_F8            0x8c    // taken from cbm petscii

#define ASCII_CURSOR_UP     0x91    // petscii cursor up
#define ASCII_REVERSE_OFF   0x92    // petscii
#define ASCII_CURSOR_LEFT   0x9d    // petscii cursor left

#endif
