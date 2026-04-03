// JKeys.h
//
// Platform-independent key types and constants for JMoria
// Replaces SDL_Keysym / SDL_Keycode / SDLK_* / KMOD_* with standalone definitions.
// Values match SDL2 so the OpenGL event path can translate without a lookup table.
//
#ifndef __JKEYS_H__
#define __JKEYS_H__

#include <cstdint>

// --- Key types ---
typedef int32_t JKeycode;
typedef uint16_t JKeymod;

struct JKeysym
{
    JKeycode sym;
    JKeymod mod;

    JKeysym() : sym( 0 ), mod( 0 ) {}
};

// --- Scancode mask (matches SDL2) ---
#define JKEY_SCANCODE_MASK ( 1 << 30 )

// --- Printable / ASCII-range key codes ---
#define JKEY_BACKSPACE '\b'
#define JKEY_TAB '\t'
#define JKEY_RETURN '\r'
#define JKEY_ESCAPE '\033'
#define JKEY_SPACE ' '
#define JKEY_COMMA ','
#define JKEY_MINUS '-'
#define JKEY_PERIOD '.'
#define JKEY_SEMICOLON ';'
#define JKEY_DELETE '\177'

// Digits
#define JKEY_0 '0'
#define JKEY_1 '1'
#define JKEY_2 '2'
#define JKEY_3 '3'
#define JKEY_4 '4'
#define JKEY_5 '5'
#define JKEY_6 '6'
#define JKEY_7 '7'
#define JKEY_8 '8'
#define JKEY_9 '9'

// Letters (lowercase, matching SDL2 convention)
#define JKEY_a 'a'
#define JKEY_b 'b'
#define JKEY_c 'c'
#define JKEY_d 'd'
#define JKEY_e 'e'
#define JKEY_f 'f'
#define JKEY_g 'g'
#define JKEY_h 'h'
#define JKEY_i 'i'
#define JKEY_j 'j'
#define JKEY_k 'k'
#define JKEY_l 'l'
#define JKEY_m 'm'
#define JKEY_n 'n'
#define JKEY_o 'o'
#define JKEY_p 'p'
#define JKEY_q 'q'
#define JKEY_r 'r'
#define JKEY_s 's'
#define JKEY_t 't'
#define JKEY_u 'u'
#define JKEY_v 'v'
#define JKEY_w 'w'
#define JKEY_x 'x'
#define JKEY_y 'y'
#define JKEY_z 'z'

// --- Scancode-based key codes (value = scancode | JKEY_SCANCODE_MASK) ---

// Function keys
#define JKEY_F1 ( 58 | JKEY_SCANCODE_MASK )

// Arrow keys
#define JKEY_RIGHT ( 79 | JKEY_SCANCODE_MASK )
#define JKEY_LEFT ( 80 | JKEY_SCANCODE_MASK )
#define JKEY_DOWN ( 81 | JKEY_SCANCODE_MASK )
#define JKEY_UP ( 82 | JKEY_SCANCODE_MASK )

// Keypad
#define JKEY_KP_1 ( 89 | JKEY_SCANCODE_MASK )
#define JKEY_KP_2 ( 90 | JKEY_SCANCODE_MASK )
#define JKEY_KP_3 ( 91 | JKEY_SCANCODE_MASK )
#define JKEY_KP_4 ( 92 | JKEY_SCANCODE_MASK )
#define JKEY_KP_5 ( 93 | JKEY_SCANCODE_MASK )
#define JKEY_KP_6 ( 94 | JKEY_SCANCODE_MASK )
#define JKEY_KP_7 ( 95 | JKEY_SCANCODE_MASK )
#define JKEY_KP_8 ( 96 | JKEY_SCANCODE_MASK )
#define JKEY_KP_9 ( 97 | JKEY_SCANCODE_MASK )
#define JKEY_KP_0 ( 98 | JKEY_SCANCODE_MASK )

// Modifier keys (as key codes, for switch/case matching)
#define JKEY_LCTRL ( 224 | JKEY_SCANCODE_MASK )
#define JKEY_LSHIFT ( 225 | JKEY_SCANCODE_MASK )
#define JKEY_RCTRL ( 228 | JKEY_SCANCODE_MASK )
#define JKEY_RSHIFT ( 229 | JKEY_SCANCODE_MASK )

// --- Key modifier flags (bitmask, for keysym.mod) ---
#define JMOD_NONE 0x0000
#define JMOD_LSHIFT 0x0001
#define JMOD_RSHIFT 0x0002
#define JMOD_LCTRL 0x0040
#define JMOD_RCTRL 0x0080
#define JMOD_SHIFT ( JMOD_LSHIFT | JMOD_RSHIFT )
#define JMOD_CTRL ( JMOD_LCTRL | JMOD_RCTRL )

#endif // __JKEYS_H__
