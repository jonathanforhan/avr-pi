/**
 * avr-pi
 * Copyright (C) 2024 Jonathan Forhan <jonathan.forhan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <avr.h>

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "avr_defs.h"
#include "defs.h"

#ifndef NDEBUG
#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wtype-limits"
#elif defined __clang__
#pragma clang diagnostic ignored "-Wtype-limits"
#endif
#endif

/*******************************************************************************
 * Arithmetic and Logic Instructions
 ******************************************************************************/

// add - add without carry
static inline void add(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R <- Rd + Rr
    const u8 R = *Rd + *Rr;

    const u8 Rd3 = GET_BIT(*Rd, 3), Rr3 = GET_BIT(*Rr, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), Rr7 = GET_BIT(*Rr, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = Rd3 & Rr3 | Rr3 & ~R3 | ~R3 & Rd3
    SET_BIT(*mcu->sreg, SREG_H, (Rd3 & Rr3) | (Rr3 & ~R3) | (~R3 & Rd3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = Rd7 & Rr7 & ~R7 | ~Rd7 & ~Rr7 & R7
    SET_BIT(*mcu->sreg, SREG_V, (Rd7 & Rr7 & ~R7) | (~Rd7 & ~Rr7 & R7));
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = Rd7 & Rr7 | Rr7 & ~R7 | ~R7 & Rd7
    SET_BIT(*mcu->sreg, SREG_C, (Rd7 & Rr7) | (Rr7 & ~R7) | (~R7 & Rd7));

    *Rd = R;
}

// adc - add with carry
static inline void adc(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R <- Rd + Rr + C
    const u8 R = *Rd + *Rr + GET_BIT(*mcu->sreg, SREG_C);

    const u8 Rd3 = GET_BIT(*Rd, 3), Rr3 = GET_BIT(*Rr, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), Rr7 = GET_BIT(*Rr, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = Rd3 & Rr3 | Rr3 & ~R3 | ~R3 & Rd3
    SET_BIT(*mcu->sreg, SREG_H, (Rd3 & Rr3) | (Rr3 & ~R3) | (~R3 & Rd3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = Rd7 & Rr7 & ~R7 | ~Rd7 & ~Rr7 & R7
    SET_BIT(*mcu->sreg, SREG_V, (Rd7 & Rr7 & ~R7) | (~Rd7 & ~Rr7 & R7));
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0 & Z
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = Rd7 & Rr7 | Rr7 & ~R7 | ~R7 & Rd7
    SET_BIT(*mcu->sreg, SREG_C, (Rd7 & Rr7) | (Rr7 & ~R7) | (~R7 & Rd7));

    *Rd = R;
}

// adiw - add immediate word
static inline void adiw(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 0, 3);
    ASSERT_BOUNDS(K, 0, 63);

    // reg pairs { 24, 26, 28, 30 }
    u16 *Rd = (u16 *)&mcu->reg[d * 2 + 24];

    // R <- Rd + Rr
    const u16 R = *Rd + K;

    const u8 Rdh7 = GET_BIT(*Rd, 15);
    const u8 R15  = GET_BIT(R, 15);

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = ~Rdh7 & R15
    SET_BIT(*mcu->sreg, SREG_V, ~Rdh7 & R15);
    // N = R15
    SET_BIT(*mcu->sreg, SREG_N, R15);
    // Z = ~R15 & ~R14 & ~R13 & ~R12 & ~R11 & ~R10 & ~R9 & ~R8 & ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = ~R15 & Rdh7
    SET_BIT(*mcu->sreg, SREG_C, ~R15 & Rdh7);

    *Rd = R;
}

// sub - subtract without carry
static inline void sub(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R <- Rd - Rr
    const u8 R = *Rd - *Rr;

    const u8 Rd3 = GET_BIT(*Rd, 3), Rr3 = GET_BIT(*Rr, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), Rr7 = GET_BIT(*Rr, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = ~Rd3 & Rr3 | Rr3 & R3 | R3 & ~Rd3
    SET_BIT(*mcu->sreg, SREG_H, (~Rd3 & Rr3) | (Rr3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = Rd7 & ~Rr7 & ~R7 | ~Rd7 & Rr7 & R7
    SET_BIT(*mcu->sreg, SREG_V, (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7));
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = ~Rd7 & Rr7 | Rr7 & R7 | R7 & ~Rd7
    SET_BIT(*mcu->sreg, SREG_C, (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7));

    *Rd = R;
}

// subi - subtract immediate
static inline void subi(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(K, 0, 255);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd - K
    const u8 R = *Rd - K;

    const u8 Rd3 = GET_BIT(*Rd, 3), K3 = GET_BIT(K, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), K7 = GET_BIT(K, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = ~Rd3 & K3 | K3 & R3 | R3 & ~Rd3
    SET_BIT(*mcu->sreg, SREG_H, (~Rd3 & K3) | (K3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = Rd7 & ~K7 & ~R7 | ~Rd7 & K7 & R7
    SET_BIT(*mcu->sreg, SREG_V, (Rd7 & ~K7 & ~R7) | (~Rd7 & K7 & R7));
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = ~Rd7 & K7 | K7 & R7 | R7 & ~Rd7
    SET_BIT(*mcu->sreg, SREG_C, (~Rd7 & K7) | (K7 & R7) | (R7 & ~Rd7));

    *Rd = R;
}

// sbc - subtract with carry
static inline void sbc(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R <- Rd - Rr - C
    const u8 R = *Rd - *Rr - GET_BIT(*mcu->sreg, SREG_C);

    const u8 Rd3 = GET_BIT(*Rd, 3), Rr3 = GET_BIT(*Rr, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), Rr7 = GET_BIT(*Rr, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = ~Rd3 & Rr3 | Rr3 & R3 | R3 & ~Rd3
    SET_BIT(*mcu->sreg, SREG_H, (~Rd3 & Rr3) | (Rr3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = Rd7 & ~Rr7 & ~R7 | ~Rd7 & Rr7 & R7
    SET_BIT(*mcu->sreg, SREG_V, (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7));
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0 & Z
    SET_BIT(*mcu->sreg, SREG_Z, R == 0 && GET_BIT(*mcu->sreg, SREG_Z));
    // C = ~Rd7 & Rr7 | Rr7 & R7 | R7 & ~Rd7
    SET_BIT(*mcu->sreg, SREG_C, (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7));

    *Rd = R;
}

// sbci - subtract immediate with carry
static inline void sbci(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(K, 0, 255);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd - K - C
    const u8 R = *Rd - K - GET_BIT(*mcu->sreg, SREG_C);

    const u8 Rd3 = GET_BIT(*Rd, 3), K3 = GET_BIT(K, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), K7 = GET_BIT(K, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = ~Rd3 & K3 | K3 & R3 | R3 & ~Rd3
    SET_BIT(*mcu->sreg, SREG_H, (~Rd3 & K3) | (K3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = Rd7 & ~K7 & ~R7 | ~Rd7 & K7 & R7
    SET_BIT(*mcu->sreg, SREG_V, (Rd7 & ~K7 & ~R7) | (~Rd7 & K7 & R7));
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0 & Z
    SET_BIT(*mcu->sreg, SREG_Z, R == 0 && GET_BIT(*mcu->sreg, SREG_Z));
    // C = ~Rd7 & K7 | K7 & R7 | R7 & ~Rd7
    SET_BIT(*mcu->sreg, SREG_C, (~Rd7 & K7) | (K7 & R7) | (R7 & ~Rd7));

    *Rd = R;
}

// sbiw - subtract immediate from word
static inline void sbiw(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 0, 3);
    ASSERT_BOUNDS(K, 0, 63);

    // reg pairs { 24, 26, 28, 30 }
    u16 *Rd = (u16 *)&mcu->reg[d * 2 + 24];

    // R <- Rd - Rr
    const u16 R = *Rd - K;

    const u8 Rdh7 = GET_BIT(*Rd, 15);
    const u8 R15  = GET_BIT(R, 15);

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = R15 & ~Rdh7
    SET_BIT(*mcu->sreg, SREG_V, R15 & ~Rdh7);
    // N = R15
    SET_BIT(*mcu->sreg, SREG_N, R15);
    // Z = ~R15 & ~R14 & ~R13 & ~R12 & ~R11 & ~R10 & ~R9 & ~R8 & ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // V = R15 & ~Rdh7
    SET_BIT(*mcu->sreg, SREG_C, R15 & ~Rdh7);

    *Rd = R;
}

// and - logical and
static inline void and (AVR_MCU *restrict mcu, u8 d, const u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R <- Rd & Rr
    *Rd = *Rd & *Rr;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(*mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, *Rd == 0);
}

// andi - logical and with immediate
static inline void andi(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(K, 0, 255);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd & K
    *Rd = *Rd & K;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(*mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, *Rd == 0);
}

// or - logical or
static inline void or (AVR_MCU *restrict mcu, u8 d, const u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R <- Rd | Rr
    *Rd = *Rd | *Rr;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(*mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, *Rd == 0);
}

// ori - logical or with immediate
static inline void ori(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(K, 0, 255);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd | K
    *Rd = *Rd | K;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(*mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, *Rd == 0);
}

// eor - exclusive or
static inline void eor(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R = Rd ^ Rr
    *Rd = *Rd ^ *Rr;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(*mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, *Rd == 0);
}

// com - one's complement
static inline void com(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R = $FF - Rd
    *Rd = ~(*Rd);

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(*mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, *Rd == 0);
    // C = 1
    PUT_BIT(*mcu->sreg, SREG_C);
}

// neg - two's complement
static inline void neg(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R <- $00 - Rd
    const u8 R = TWO_COMP(*Rd);

    // PC <- PC + 1
    mcu->pc++;

    // H = R3 & ~Rd3
    SET_BIT(*mcu->sreg, SREG_H, GET_BIT(R, 3) & ~GET_BIT(*Rd, 3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_V, R == 0x80);
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(R, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = R7 | R6 | R5 | R4 | R3 | R2 | R1 | R0
    SET_BIT(*mcu->sreg, SREG_C, R != 0);

    *Rd = R;
}

// inc - increment
static inline void inc(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R = Rd + 1
    *Rd = *Rd + 1;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = ~R7 & R6 & R5 & R4 & R3 & R2 & R1 & R0
    SET_BIT(*mcu->sreg, SREG_V, *Rd == 0x80);
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, *Rd == 0);
}

// dec - decrement
static inline void dec(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R = Rd - 1
    *Rd = *Rd - 1;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = ~R7 & R6 & R5 & R4 & R3 & R2 & R1 & R0
    SET_BIT(*mcu->sreg, SREG_V, *Rd == 0x7F);
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, *Rd == 0);
}

// ser - set all bits in register
static inline void ser(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 16, 31);

    u8 *Rd = &mcu->reg[d];

    // R <- $FF
    *Rd = 0xFF;

    // PC <- PC + 1
    mcu->pc++;
}

// mul - multiply unsigned
static inline void mul(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R1:R0 <- Rd * Rr
    const u16 R = (u16)*Rd * (u16)*Rr;

    // PC <- PC + 1
    mcu->pc++;

    // C = R15
    SET_BIT(*mcu->sreg, SREG_C, GET_BIT(R, 15));
    // Z = ~R15 & ~R14 & ~R13 & ~R12 & ~R11 & ~R10 & ~R9 & ~R8 & ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);

    *(u16 *)&mcu->reg[0] = R;
}

// muls - multiply signed
static inline void muls(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(r, 16, 31);

    i8 *Rd       = (i8 *)&mcu->reg[d];
    const i8 *Rr = (i8 *)&mcu->reg[r];

    // R1:R0 <- Rd * Rr
    const i16 R = (i16)(*Rd * *Rr);

    // PC <- PC + 1
    mcu->pc++;

    // C = R15
    SET_BIT(*mcu->sreg, SREG_C, GET_BIT(R, 15));
    // Z = ~R15 & ~R14 & ~R13 & ~R12 & ~R11 & ~R10 & ~R9 & ~R8 & ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);

    *(i16 *)&mcu->reg[0] = R;
}

// mulsu - multiply signed with unsigned
static inline void mulsu(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 16, 23);
    ASSERT_BOUNDS(r, 16, 23);

    i8 *Rd       = (i8 *)&mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R1:R0 <- Rd * Rr
    const i16 R = (i16)(*Rd * *Rr);

    // PC <- PC + 1
    mcu->pc++;

    // C = R15
    SET_BIT(*mcu->sreg, SREG_C, GET_BIT(R, 15));
    // Z = ~R15 & ~R14 & ~R13 & ~R12 & ~R11 & ~R10 & ~R9 & ~R8 & ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);

    *(i16 *)&mcu->reg[0] = R;
}

// fmul - fractional multiply unsigned
static inline void fmul(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 16, 23);
    ASSERT_BOUNDS(r, 16, 23);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R1:R0 <- Rd * Rr << 1
    const u32 R = (*Rd * *Rr) << 1;

    // PC <- PC + 1
    mcu->pc++;

    // C = R16
    SET_BIT(*mcu->sreg, SREG_C, GET_BIT(R, 16));
    // Z = ~R15 & ~R14 & ~R13 & ~R12 & ~R11 & ~R10 & ~R9 & ~R8 & ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);

    *(u16 *)&mcu->reg[0] = (u16)R;
}

// fmuls - fractional multiply signed
static inline void fmuls(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 16, 23);
    ASSERT_BOUNDS(r, 16, 23);

    i8 *Rd       = (i8 *)&mcu->reg[d];
    const i8 *Rr = (i8 *)&mcu->reg[r];

    // R1:R0 <- Rd * Rr << 1
    const i32 R = (*Rd * *Rr) << 1;

    // PC <- PC + 1
    mcu->pc++;

    // C = R16
    SET_BIT(*mcu->sreg, SREG_C, GET_BIT(R, 16));
    // Z = ~R15 & ~R14 & ~R13 & ~R12 & ~R11 & ~R10 & ~R9 & ~R8 & ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);

    *(i16 *)&mcu->reg[0] = (i16)R;
}

// fmulsu - fractional multiply signed with unsigned
static inline void fmulsu(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 16, 23);
    ASSERT_BOUNDS(r, 16, 23);

    i8 *Rd       = (i8 *)&mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R1:R0 <- Rd * Rr << 1
    const i32 R = (*Rd * *Rr) << 1;

    // PC <- PC + 1
    mcu->pc++;

    // C = R16
    SET_BIT(*mcu->sreg, SREG_C, GET_BIT(R, 16));
    // Z = ~R15 & ~R14 & ~R13 & ~R12 & ~R11 & ~R10 & ~R9 & ~R8 & ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);

    *(i16 *)&mcu->reg[0] = (i16)R;
}

/*******************************************************************************
 * Branch Instructions
 ******************************************************************************/

// rjmp - relative jump
static inline void rjmp(AVR_MCU *restrict mcu, i16 k) {
    ASSERT_BOUNDS(k, -2048, 2047);

    // PC <- PC + k + 1
    mcu->pc += k + 1;
}

// ijmp - indirect jump
static inline void ijmp(AVR_MCU *restrict mcu) {
    // PC(15:0) <- Z(15:0)
    mcu->pc = *(u16 *)&mcu->reg[REG_Z];
}

// jmp - jump
static inline void jmp(AVR_MCU *restrict mcu, u16 k) {
    ASSERT_BOUNDS(k, 0, sizeof(mcu->flash) - 1);

    // PC <- k
    mcu->pc = k;
}

// rcall - relative call
static inline void rcall(AVR_MCU *restrict mcu, i16 k) {
    ASSERT_BOUNDS(k, -2048, 2047);

    // SP <- SP - 2
    *mcu->sp -= 2;

    // STACK <- PC + 1
    *(u16 *)&mcu->data[*mcu->sp] = mcu->pc + 1;

    // PC <- PC + k + 1
    mcu->pc += k + 1;
}

// icall indirect call to subroutine
static inline void icall(AVR_MCU *restrict mcu) {
    // SP <- SP - 2
    *mcu->sp -= 2;

    // STACK <- PC + 1
    *(u16 *)&mcu->data[*mcu->sp] = mcu->pc + 1;

    // PC(15:0) <- Z(15:0)
    mcu->pc = *(u16 *)&mcu->reg[REG_Z];
}

// call - long call to a subroutine
static inline void call(AVR_MCU *restrict mcu, u16 k) {
    ASSERT_BOUNDS(k, 0, sizeof(mcu->flash) - 1);

    // SP <- PC - 2
    *mcu->sp -= 2;

    // STACK <- PC + 2
    *(u16 *)&mcu->data[*mcu->sp] = mcu->pc + 2;

    // PC <- k
    mcu->pc = k;
}

// ret - return from subroutine
static inline void ret(AVR_MCU *restrict mcu) {
    // PC(15:0) <- STACK
    mcu->pc = *(u16 *)&mcu->data[*mcu->sp];

    // SP <- PC + 2
    *mcu->sp += 2;
}

// reti - return from interrupt
static inline void reti(AVR_MCU *restrict mcu) {
    // PC(15:0) <- STACK
    mcu->pc = *(u16 *)&mcu->data[*mcu->sp];

    // SP <- PC + 2
    *mcu->sp += 2;

    // I = 1
    PUT_BIT(*mcu->sreg, SREG_I);
}

// cpse - compare skip if equal
static inline void cpse(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rd = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // if Rd == Rr then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (*Rd == *Rr) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        // PC <- PC + 2 (or 3)
        mcu->pc += 2 + IS_32BIT_OP(next_op);
    } else {
        // PC <- PC + 1
        mcu->pc++;
    }
}

// cp - compare
static inline void cp(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rd = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R = Rd - Rr
    const u8 R = *Rd - *Rr;

    const u8 Rd3 = GET_BIT(*Rd, 3), Rr3 = GET_BIT(*Rr, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), Rr7 = GET_BIT(*Rr, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = ~Rd3 & Rr3 | Rr3 & R3 | R3 & ~Rd3
    SET_BIT(*mcu->sreg, SREG_H, (~Rd3 & Rr3) | (Rr3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = Rd7 & ~Rr7 & ~R7 | ~Rd7 & Rr7 & R7
    SET_BIT(*mcu->sreg, SREG_V, (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7));
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = ~Rd7 & Rr7 | Rr7 & R7 | R7 & ~Rd7
    SET_BIT(*mcu->sreg, SREG_C, (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7));
}

// cpc - compare with carry
static inline void cpc(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rd = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R = Rd - Rr - C
    const u8 R = *Rd - *Rr - GET_BIT(*mcu->sreg, SREG_C);

    const u8 Rd3 = GET_BIT(*Rd, 3), Rr3 = GET_BIT(*Rr, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), Rr7 = GET_BIT(*Rr, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = ~Rd3 & Rr3 | Rr3 & R3 | R3 & ~Rd3
    SET_BIT(*mcu->sreg, SREG_H, (~Rd3 & Rr3) | (Rr3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = Rd7 & ~Rr7 & ~R7 | ~Rd7 & Rr7 & R7
    SET_BIT(*mcu->sreg, SREG_V, (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7));
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0 & Z
    SET_BIT(*mcu->sreg, SREG_Z, R == 0 && GET_BIT(*mcu->sreg, SREG_Z));
    // C = ~Rd7 & Rr7 | Rr7 & R7 | R7 & ~Rd7
    SET_BIT(*mcu->sreg, SREG_C, (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7));
}

// cpi - compare with immediate
static inline void cpi(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(K, 0, 255);

    const u8 *Rd = &mcu->reg[d];

    // R = Rd - K
    const u8 R = *Rd - K;

    const u8 Rd3 = GET_BIT(*Rd, 3), K3 = GET_BIT(K, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), K7 = GET_BIT(K, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = ~Rd3 & K3 | K3 & R3 | R3 & ~Rd3
    SET_BIT(*mcu->sreg, SREG_H, (~Rd3 & K3) | (K3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = Rd7 & ~K7 & ~R7 | ~Rd7 & K7 & R7
    SET_BIT(*mcu->sreg, SREG_V, (Rd7 & ~K7 & ~R7) | (~Rd7 & K7 & R7));
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = ~Rd7 & K7 | K7 & R7 | R7 & ~Rd7
    SET_BIT(*mcu->sreg, SREG_C, (~Rd7 & K7) | (K7 & R7) | (R7 & ~Rd7));
}

// sbrc - skip if bit in register is cleared
static inline void sbrc(AVR_MCU *restrict mcu, u8 r, u8 b) {
    ASSERT_BOUNDS(r, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    const u8 *Rr = &mcu->reg[r];

    // if Rr(b) = 0 then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (GET_BIT(*Rr, b) == 0) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        // PC <- PC + 2 (or 3)
        mcu->pc += 2 + IS_32BIT_OP(next_op);
    } else {
        // PC <- PC + 1
        mcu->pc++;
    }
}

// sbrs - skip if bit in register is set
static inline void sbrs(AVR_MCU *restrict mcu, u8 r, u8 b) {
    ASSERT_BOUNDS(r, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    const u8 *Rr = &mcu->reg[r];

    // if Rr(b) = 1 then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (GET_BIT(*Rr, b)) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        // PC <- PC + 2 (or 3)
        mcu->pc += 2 + IS_32BIT_OP(next_op);
    } else {
        // PC <- PC + 1
        mcu->pc++;
    }
}

// sbic - skip if bit in io register is cleared
static inline void sbic(AVR_MCU *restrict mcu, u8 A, u8 b) {
    ASSERT_BOUNDS(A, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    // if IO(A,b) = 0 then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (GET_BIT(mcu->io_reg[A], b) == 0) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        // PC <- PC + 2 (or 3)
        mcu->pc += 2 + IS_32BIT_OP(next_op);
    } else {
        // PC <- PC + 1
        mcu->pc++;
    }
}

// sbis - skip if bit in io register is set
static inline void sbis(AVR_MCU *restrict mcu, u8 A, u8 b) {
    ASSERT_BOUNDS(A, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    // if Rr(b) = 1 then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (GET_BIT(mcu->io_reg[A], b)) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        // PC <- PC + 2 (or 3)
        mcu->pc += 2 + IS_32BIT_OP(next_op);
    } else {
        // PC <- PC + 1
        mcu->pc++;
    }
}

// brbs - branch if bit in sreg is set
static inline void brbs(AVR_MCU *restrict mcu, u8 s, i8 k) {
    ASSERT_BOUNDS(s, 0, 7);
    ASSERT_BOUNDS(k, -64, 63);

    // PC <- PC + k + 1 if true
    // PC <- PC + 1 if false
    if (GET_BIT(*mcu->sreg, s) == 1) {
        mcu->pc += k + 1;
    } else {
        mcu->pc++;
    }
}

// brbc - branch if bit in sreg is cleared
static inline void brbc(AVR_MCU *restrict mcu, u8 s, i8 k) {
    ASSERT_BOUNDS(s, 0, 7);
    ASSERT_BOUNDS(k, -64, 63);

    // PC <- PC + k + 1 if true
    // PC <- PC + 1 if false
    if (GET_BIT(*mcu->sreg, s) == 0) {
        mcu->pc += k + 1;
    } else {
        mcu->pc++;
    }
}

/*******************************************************************************
 * Bit and Bit-Test Instructions
 ******************************************************************************/

// sbi - set bit in io register
static inline void sbi(AVR_MCU *restrict mcu, u8 A, u8 b) {
    ASSERT_BOUNDS(A, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    // IO(A,b) = 1
    PUT_BIT(mcu->io_reg[A], b);

    // PC <- PC + 1
    mcu->pc++;
}

// cbi - clear bit in io register
static inline void cbi(AVR_MCU *restrict mcu, u8 A, u8 b) {
    ASSERT_BOUNDS(A, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    // IO(A,b) = 0
    CLR_BIT(mcu->io_reg[A], b);

    // PC <- PC + 1
    mcu->pc++;
}

// lsr - logical shift right
static inline void lsr(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd >> 1
    const u8 R = *Rd >> 1;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = N ^ C  **DELAYED**
    /* see NOTE */
    // N = 0
    CLR_BIT(*mcu->sreg, SREG_N);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = Rd0
    SET_BIT(*mcu->sreg, SREG_C, GET_BIT(*Rd, 0));

    // V = N ^ C  <- NOTE N and C AFTER shift so we delay it
    SET_BIT(*mcu->sreg, SREG_V, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_C));

    *Rd = R;
}

// ror - rotate right through carry
static inline void ror(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R <- C -> Rd >> 1
    const u8 R = (*Rd >> 1) | (GET_BIT(*mcu->sreg, SREG_C) << 7);

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = N ^ C  **DELAYED**
    /* see NOTE */
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(R, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = Rd0
    SET_BIT(*mcu->sreg, SREG_C, GET_BIT(*Rd, 0));

    // V = N ^ C  <- NOTE N and C AFTER shift so we delay it
    SET_BIT(*mcu->sreg, SREG_V, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_C));

    *Rd = R;
}

// asr - arithmetic shift right
static inline void asr(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd >> 1
    const u8 R = ((i8)*Rd) >> 1;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(*mcu->sreg, SREG_S, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_V));
    // V = N ^ C  **DELAYED**
    /* see NOTE */
    // N = R7
    SET_BIT(*mcu->sreg, SREG_N, GET_BIT(R, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(*mcu->sreg, SREG_Z, R == 0);
    // C = Rd0
    SET_BIT(*mcu->sreg, SREG_C, GET_BIT(*Rd, 0));

    // V = N ^ C  <- NOTE N and C AFTER shift so we delay it
    SET_BIT(*mcu->sreg, SREG_V, GET_BIT(*mcu->sreg, SREG_N) ^ GET_BIT(*mcu->sreg, SREG_C));

    *Rd = R;
}

// swap - swap nibbles
static inline void swap(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R(7:4) = Rd(3:0), R(3:0) = Rd(7:4)
    const u8 R = (*Rd >> 4) | (*Rd << 4);

    // PC <- PC + 1
    mcu->pc++;

    *Rd = R;
}

// bset - bit set in sreg
static inline void bset(AVR_MCU *restrict mcu, u8 s) {
    ASSERT_BOUNDS(s, 0, 7);

    // SREG(s) <- 1
    PUT_BIT(*mcu->sreg, s);

    // PC <- PC + 1
    mcu->pc++;

    // I = 1 if s == 7; unchanged otherwise.
    // T = 1 if s == 6; unchanged otherwise.
    // H = 1 if s == 5; unchanged otherwise.
    // S = 1 if s == 4; unchanged otherwise.
    // V = 1 if s == 3; unchanged otherwise.
    // N = 1 if s == 2; unchanged otherwise.
    // Z = 1 if s == 1; unchanged otherwise.
    // C = 1 if s == 0; unchanged otherwise.
}

// bclr - bit clear in sreg
static inline void bclr(AVR_MCU *restrict mcu, u8 s) {
    ASSERT_BOUNDS(s, 0, 7);

    // SREG(s) <- 0
    CLR_BIT(*mcu->sreg, s);

    // PC <- PC + 1
    mcu->pc++;

    // I = 0 if s == 7; unchanged otherwise.
    // T = 0 if s == 6; unchanged otherwise.
    // H = 0 if s == 5; unchanged otherwise.
    // S = 0 if s == 4; unchanged otherwise.
    // V = 0 if s == 3; unchanged otherwise.
    // N = 0 if s == 2; unchanged otherwise.
    // Z = 0 if s == 1; unchanged otherwise.
    // C = 0 if s == 0; unchanged otherwise.
}

// bst - bit store from bit in register to T flag in sreg
static inline void bst(AVR_MCU *restrict mcu, u8 r, u8 b) {
    ASSERT_BOUNDS(r, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    const u8 *Rr = &mcu->reg[r];

    // T <- Rr(b)
    SET_BIT(*mcu->sreg, SREG_T, GET_BIT(*Rr, b));

    // PC <- PC + 1
    mcu->pc++;

    // T = 0 if bit b in Rr is cleared. Set to 1 otherwise.
}

// bld - bit load from the T flag in sreg to a bit in register
static inline void bld(AVR_MCU *restrict mcu, u8 d, u8 b) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    u8 *Rd = &mcu->reg[d];

    // Rd(b) <- T
    SET_BIT(*Rd, b, GET_BIT(*mcu->sreg, SREG_T));

    // PC <- PC + 1
    mcu->pc++;
}

/*******************************************************************************
 * Data Transfer Instructions
 ******************************************************************************/

// mov - copy register
static inline void mov(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // Rd <- Rr
    *Rd = *Rr;

    // PC <- PC + 1
    mcu->pc++;
}

// movw - copy register word
static inline void movw(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 30);
    ASSERT_BOUNDS(r, 0, 30);

    u16 *Rd       = (u16 *)&mcu->reg[d];
    const u16 *Rr = (u16 *)&mcu->reg[r];

    // Rd <- Rr
    *Rd = *Rr;

    // PC <- PC + 1
    mcu->pc++;
}

// ldi - load immediate
static inline void ldi(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(d, 0, 255);

    u8 *Rd = &mcu->reg[d];

    // Rd <- K
    *Rd = K;

    // PC <- PC + 1
    mcu->pc++;
}

// ld - load indirect from data space to register using index X
static inline void ld_x(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd      = &mcu->reg[d];
    const u16 X = *(u16 *)&mcu->reg[REG_X];

    // Rd <- (X)
    *Rd = mcu->data[X];

    // PC <- PC + 1
    mcu->pc++;
}

static inline void ld_x_postinc(AVR_MCU *restrict mcu, u8 d) {
    ld_x(mcu, d);
    (*(u16 *)&mcu->reg[REG_X])++;
}

static inline void ld_x_predec(AVR_MCU *restrict mcu, u8 d) {
    (*(u16 *)&mcu->reg[REG_X])--;
    ld_x(mcu, d);
}

static inline void ld_y(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd      = &mcu->reg[d];
    const u16 Y = *(u16 *)&mcu->reg[REG_Y];

    // Rd <- (Y)
    *Rd = mcu->data[Y];

    // PC <- PC + 1
    mcu->pc++;
}

static inline void ld_y_postinc(AVR_MCU *restrict mcu, u8 d) {
    ld_y(mcu, d);
    (*(u16 *)&mcu->reg[REG_Y])++;
}

static inline void ld_y_predec(AVR_MCU *restrict mcu, u8 d) {
    (*(u16 *)&mcu->reg[REG_Y])--;
    ld_y(mcu, d);
}

static inline void ld_z(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd      = &mcu->reg[d];
    const u16 Z = *(u16 *)&mcu->reg[REG_Z];

    // Rd <- (Z)
    *Rd = mcu->data[Z];

    // PC <- PC + 1
    mcu->pc++;
}

static inline void ld_z_postinc(AVR_MCU *restrict mcu, u8 d) {
    ld_z(mcu, d);
    (*(u16 *)&mcu->reg[REG_Z])++;
}

static inline void ld_z_predec(AVR_MCU *restrict mcu, u8 d) {
    (*(u16 *)&mcu->reg[REG_Z])--;
    ld_z(mcu, d);
}

// ldd
static inline void ldd_y(AVR_MCU *restrict mcu, u8 d, u8 q) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(q, 0, 63);

    u8 *Rd      = &mcu->reg[d];
    const u16 Y = *(u16 *)&mcu->reg[REG_Y + q];

    // Rd <- (Y)
    *Rd = mcu->data[Y];

    // PC <- PC + 1
    mcu->pc++;
}

static inline void ldd_z(AVR_MCU *restrict mcu, u8 d, u8 q) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(q, 0, 63);

    u8 *Rd      = &mcu->reg[d];
    const u16 Z = *(u16 *)&mcu->reg[REG_Z + q];

    // Rd <- (Z)
    *Rd = mcu->data[Z];

    // PC <- PC + 1
    mcu->pc++;
}

// lds - load direct from data space
static inline void lds(AVR_MCU *restrict mcu, u8 d, u16 k) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(k, 0, sizeof(mcu->data) - 1);

    u8 *Rd = &mcu->reg[d];

    // Rd <- (k)
    *Rd = mcu->data[k];

    // PC <- PC + 2
    mcu->pc += 2;
}

// st - store indirect from register to data space using index X
static inline void st_x(AVR_MCU *restrict mcu, u8 r) {
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 X  = *(u16 *)&mcu->reg[REG_X];

    // (X) <- Rr
    mcu->data[X] = *Rr;

    // PC <- PC + 1
    mcu->pc++;
}

static inline void st_x_postinc(AVR_MCU *restrict mcu, u8 r) {
    st_x(mcu, r);
    (*(u16 *)&mcu->reg[REG_X])++;
}

static inline void st_x_predec(AVR_MCU *restrict mcu, u8 r) {
    (*(u16 *)&mcu->reg[REG_X])--;
    st_x(mcu, r);
}

static inline void st_y(AVR_MCU *restrict mcu, u8 r) {
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 Y  = *(u16 *)&mcu->reg[REG_Y];

    // (Y) <- Rr
    mcu->data[Y] = *Rr;

    // PC <- PC + 1
    mcu->pc++;
}

static inline void st_y_postinc(AVR_MCU *restrict mcu, u8 r) {
    st_y(mcu, r);
    (*(u16 *)&mcu->reg[REG_Y])++;
}

static inline void st_y_predec(AVR_MCU *restrict mcu, u8 r) {
    (*(u16 *)&mcu->reg[REG_Y])--;
    st_y(mcu, r);
}

static inline void st_z(AVR_MCU *restrict mcu, u8 r) {
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 Z  = *(u16 *)&mcu->reg[REG_Z];

    // (Z) <- Rr
    mcu->data[Z] = *Rr;

    // PC <- PC + 1
    mcu->pc++;
}

static inline void st_z_postinc(AVR_MCU *restrict mcu, u8 r) {
    st_z(mcu, r);
    (*(u16 *)&mcu->reg[REG_Z])++;
}

static inline void st_z_predec(AVR_MCU *restrict mcu, u8 r) {
    (*(u16 *)&mcu->reg[REG_Z])--;
    st_z(mcu, r);
}

// std
static inline void std_y(AVR_MCU *restrict mcu, u8 q, u8 r) {
    ASSERT_BOUNDS(q, 0, 63);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 Y  = *(u16 *)&mcu->reg[REG_Y + q];

    // (Y) <- Rr
    mcu->data[Y] = *Rr;

    // PC <- PC + 1
    mcu->pc++;
}

static inline void std_z(AVR_MCU *restrict mcu, u8 q, u8 r) {
    ASSERT_BOUNDS(q, 0, 63);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 Z  = *(u16 *)&mcu->reg[REG_Z + q];

    // (Z) <- Rr
    mcu->data[Z] = *Rr;

    // PC <- PC + 1
    mcu->pc++;
}

// sts - store direct to data space
static inline void sts(AVR_MCU *restrict mcu, u16 k, u8 r) {
    ASSERT_BOUNDS(k, 0, sizeof(mcu->data) - 1);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];

    // (k) <- Rr
    mcu->data[k] = *Rr;

    // PC <- PC + 2
    mcu->pc += 2;
}

// lpm - load program memory
static inline void lpm(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd      = &mcu->reg[d];
    const u16 Z = *(u16 *)&mcu->reg[REG_Z];

    // Rd <- (Z)
    *Rd = ((u8 *)&mcu->flash)[Z];

    // PC <- PC + 1
    mcu->pc += 1;
}

static inline void lpm_postinc(AVR_MCU *restrict mcu, u8 d) {
    lpm(mcu, d);
    (*(u16 *)&mcu->reg[REG_Z])++;
}

// spm - store program memory
static inline void spm(AVR_MCU *restrict mcu) {
    const u16 *Rr = (u16 *)&mcu->reg[0];
    const u16 Z   = *(u16 *)&mcu->reg[REG_Z];

    // (Z) <- Rr
    mcu->flash[Z] = *Rr;

    // PC <- PC + 1
    mcu->pc += 1;
}

// in - load an io location to register
static inline void in(AVR_MCU *restrict mcu, u8 d, u8 A) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(A, 0, 63);

    u8 *Rd = &mcu->reg[d];

    // Rd <- IO(A)
    *Rd = mcu->io_reg[A];

    // PC <- PC + 1
    mcu->pc++;
}

// out - store register to io location
static inline void out(AVR_MCU *restrict mcu, u8 A, u8 r) {
    ASSERT_BOUNDS(A, 0, 63);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];

    // IO(A) <- Rr
    mcu->io_reg[A] = *Rr;

    // PC <- PC + 1
    mcu->pc++;
}

// push - push register on stack
static inline void push(AVR_MCU *restrict mcu, u8 r) {
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];

    // SP <- SP - 1
    (*mcu->sp)--;

    // STACK <- Rr
    mcu->data[*mcu->sp] = *Rr;

    // PC <- PC + 1
    mcu->pc++;
}

// pop - pop register from stack
static inline void pop(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // Rd <- STACK
    *Rd = mcu->data[*mcu->sp];

    // SP <- SP + 1
    (*mcu->sp)++;

    // PC <- PC + 1
    mcu->pc++;
}

/*******************************************************************************
 * Data Transfer Instructions
 ******************************************************************************/

// nop - no operation
static inline void nop(AVR_MCU *restrict mcu) {
    mcu->pc++;
}

// sleep -
static inline void sleep(AVR_MCU *restrict mcu) {
    // TODO maybe
    mcu->pc++;
}

// wdr - watchdog reset
static inline void wdr(AVR_MCU *restrict mcu) {
    // TODO maybe
    mcu->pc++;
}

// break - break
static inline void break_(AVR_MCU *restrict mcu) {
    mcu->pc++;
}

static inline u8 xstr2byte(const char *restrict s) {
    u8 b = 0;
    b |= (isdigit(s[0]) ? s[0] - '0' : isupper(s[0]) ? s[0] - 'A' + 10 : s[0] - 'a' + 10) << 4;
    b |= (isdigit(s[1]) ? s[1] - '0' : isupper(s[1]) ? s[1] - 'A' + 10 : s[1] - 'a' + 10);
    return b;
}

void avr_mcu_init(AVR_MCU *restrict mcu) {
    memset(mcu, 0, sizeof(*mcu));

    mcu->sreg       = &mcu->data[AVR_MCU_SREG_OFFSET];
    mcu->sp         = (u16 *)&mcu->data[AVR_MCU_SP_OFFSET];
    mcu->reg        = &mcu->data[AVR_MCU_REG_OFFSET];
    mcu->io_reg     = &mcu->data[AVR_MCU_IO_REG_OFFSET];
    mcu->ext_io_reg = &mcu->data[AVR_MCU_EXT_IO_REG_OFFSET];
    mcu->sram       = &mcu->data[AVR_MCU_SRAM_OFFSET];

    *mcu->sp = AVR_MCU_RAMEND;
}

AVR_Result avr_program(AVR_MCU *restrict mcu, const char *restrict hex) {
    while (*hex) {
        if (*hex != ':') {
            hex++;
            continue;
        }

        hex++;

        const u8 len = xstr2byte(hex);
        hex += 2;

        const u16 addr = (xstr2byte(hex) << 8) | xstr2byte(hex + 2);
        hex += 4;

        const u8 type = xstr2byte(hex);
        hex += 2;

        switch (type) {
        case DATA_RECORD:
        case EXTENDED_SEGMENT_ADDR_RECORD:
        case START_SEGMENT_ADDR_RECORD:
        case EXTENDED_LINEAR_ADDR_RECORD:
        case START_LINEAR_ADDR_RECORD: {
            u8 checksum = len + (addr >> 8) + (addr & 0xFF) + type;

            for (u8 i = 0; i < len; i++) {
                const u8 b = xstr2byte(hex);
                hex += 2;

                ((u8 *)mcu->flash)[addr + i] = b;
                checksum += b;
            }

            checksum = TWO_COMP(checksum);

            if (checksum != xstr2byte(hex)) {
                LOG_ERROR("checksum failure: real %#x, expected %#x", checksum, xstr2byte(hex));
                return AVR_ERROR;
            }
            hex += 2;
        } break;
        case EOF_RECORD:
            return AVR_OK;
        default:
            LOG_ERROR("unknown record type");
            return AVR_ERROR;
        }
    }

    return AVR_ERROR;
}

void avr_cycle(AVR_MCU *const restrict mcu) {
    ASSERT_BOUNDS(*mcu->sp, 0, AVR_MCU_DATA_SIZE - 1);
    ASSERT_BOUNDS(mcu->pc, 0, AVR_MCU_FLASH_SIZE - 1);

    const u16 op = mcu->flash[mcu->pc];

    /***************************************************************************
     * 4 bit op
     **************************************************************************/
    switch (op & OP_MASK_4) {
    case OP_SUBI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,%d", "subi", d, K);
        subi(mcu, d, K);
        return;
    }
    case OP_SBCI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,%d", "sbci", d, K);
        sbci(mcu, d, K);
        return;
    }
    case OP_ANDI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,%d", "andi", d, K);
        andi(mcu, d, K);
        return;
    }
    case OP_ORI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,%d", "ori", d, K);
        ori(mcu, d, K);
        return;
    }
    case OP_RJMP: {
        const i16 k = I12_TO_I16(MSK(op, 0x0FFF));
        PRINT_DEBUG("%-6s %d", "rjmp", k);
        rjmp(mcu, k);
        return;
    }
    case OP_RCALL: {
        const i16 k = I12_TO_I16(MSK(op, 0x0FFF));
        PRINT_DEBUG("%-6s %d", "rcall", k);
        rcall(mcu, k);
        return;
    }
    case OP_CPI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,%d", "cpi", d, K);
        cpi(mcu, d, K);
        return;
    }
    case OP_LDI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,%d", "ldi", d, K);
        ldi(mcu, d, K);
        return;
    }
    }

    /***************************************************************************
     * 5 bit op
     **************************************************************************/
    switch (op & OP_MASK_5) {
    case OP_IN: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 A = MSH(op, 0x0600, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,%d", "in", d, A);
        in(mcu, d, A);
        return;
    }
    case OP_OUT: {
        const u8 A = MSH(op, 0x0600, 5) | MSK(op, 0x000F);
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s %d,r%d", "out", A, r);
        out(mcu, A, r);
        return;
    }
    }

    /***************************************************************************
     * 6 bit op
     **************************************************************************/
    switch (op & OP_MASK_6) {
    case OP_ADD: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "add", d, r);
        add(mcu, d, r);
        return;
    }
    case OP_ADC: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "adc", d, r);
        adc(mcu, d, r);
        return;
    }
    case OP_SUB: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "sub", d, r);
        sub(mcu, d, r);
        return;
    }
    case OP_SBC: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "sbc", d, r);
        sbc(mcu, d, r);
        return;
    }
    case OP_AND: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "and", d, r);
        and(mcu, d, r);
        return;
    }
    case OP_OR: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "or", d, r);
        or (mcu, d, r);
        return;
    }
    case OP_EOR: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "eor", d, r);
        eor(mcu, d, r);
        return;
    }
    case OP_MUL: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "mul", d, r);
        mul(mcu, d, r);
        return;
    }
    case OP_CPSE: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "cpse", d, r);
        cpse(mcu, d, r);
        return;
    }
    case OP_CP: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "cp", d, r);
        cp(mcu, d, r);
        return;
    }
    case OP_CPC: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "cpc", d, r);
        cpc(mcu, d, r);
        return;
    }
    case OP_BRBC: {
        const u8 s = MSK(op, 0x0003);
        const i8 k = I7_TO_I16(MSH(op, 0x03F8, 3));
        PRINT_DEBUG("%-6s %d,%d", "brbc", s, k);
        brbc(mcu, s, k);
        return;
    }
    case OP_BRBS: {
        const u8 s = MSK(op, 0x0003);
        const i8 k = I7_TO_I16(MSH(op, 0x03F8, 3));
        PRINT_DEBUG("%-6s %d,%d", "brbs", s, k);
        brbs(mcu, s, k);
        return;
    }
    case OP_MOV: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,r%d", "mov", d, r);
        mov(mcu, d, r);
        return;
    }
    }

    /***************************************************************************
     * 7 bit op
     **************************************************************************/
    switch (op & OP_MASK_7_1) {
    case OP_SBRC: {
        const u8 r = MSH(op, 0x01F0, 4);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-6s r%d,%d", "sbrc", r, b);
        sbrc(mcu, r, b);
        return;
    }
    case OP_SBRS: {
        const u8 r = MSH(op, 0x01F0, 4);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-6s r%d,%d", "sbrs", r, b);
        sbrs(mcu, r, b);
        return;
    }
    case OP_BST: {
        const u8 r = MSH(op, 0x01F0, 4);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-6s r%d,%d", "bst", r, b);
        bst(mcu, r, b);
        return;
    }
    case OP_BLD: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-6s r%d,%d", "bld", d, b);
        bld(mcu, d, b);
        return;
    }
    }

    switch (op & OP_MASK_7_3) {
    case OP_JMP: {
        const u16 k = mcu->flash[mcu->pc + 1]; // works because address space fits in 16bits
        PRINT_DEBUG("%-6s %d", "jmp", k);
        jmp(mcu, k);
        return;
    }
    case OP_CALL: {
        const u16 k = mcu->flash[mcu->pc + 1]; // works because address space fits in 16bits
        PRINT_DEBUG("%-6s %d", "call", k);
        call(mcu, k);
        return;
    }
    }

    switch (op & OP_MASK_7_4) {
    case OP_COM: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "com", d);
        com(mcu, d);
        return;
    }
    case OP_NEG: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "neg", d);
        neg(mcu, d);
        return;
    }
    case OP_INC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "inc", d);
        inc(mcu, d);
        return;
    }
    case OP_DEC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "dec", d);
        dec(mcu, d);
        return;
    }
    case OP_LSR: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "lsr", d);
        lsr(mcu, d);
        return;
    }
    case OP_ROR: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ror", d);
        ror(mcu, d);
        return;
    }
    case OP_ASR: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "asr", d);
        asr(mcu, d);
        return;
    }
    case OP_SWAP: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "swap", d);
        swap(mcu, d);
        return;
    }
    case OP_LD_X: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ld(X)", d);
        ld_x(mcu, d);
        return;
    }
    case OP_LD_X_POSTINC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ld(X+)", d);
        ld_x_postinc(mcu, d);
        return;
    }
    case OP_LD_X_PREDEC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ld(-X)", d);
        ld_x_predec(mcu, d);
        return;
    }
    case OP_LD_Y: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ld(Y)", d);
        ld_y(mcu, d);
        return;
    }
    case OP_LD_Y_POSTINC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ld(Y+)", d);
        ld_y_postinc(mcu, d);
        return;
    }
    case OP_LD_Y_PREDEC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ld(-Y)", d);
        ld_y_predec(mcu, d);
        return;
    }
    case OP_LD_Z: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ld(Z)", d);
        ld_z(mcu, d);
        return;
    }
    case OP_LD_Z_POSTINC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ld(Z+)", d);
        ld_z_postinc(mcu, d);
        return;
    }
    case OP_LD_Z_PREDEC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "ld(-Z)", d);
        ld_z_predec(mcu, d);
        return;
    }
    case OP_LDS: {
        const u8 d  = MSH(op, 0x01F0, 4);
        const u16 k = mcu->flash[mcu->pc + 1];
        PRINT_DEBUG("%-6s r%d", "lds", d);
        lds(mcu, d, k);
        return;
    }
    case OP_ST_X: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "st(X)", r);
        st_x(mcu, r);
        return;
    }
    case OP_ST_X_POSTINC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "st(X+)", r);
        st_x_postinc(mcu, r);
        return;
    }
    case OP_ST_X_PREDEC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "st(-X)", r);
        st_x_predec(mcu, r);
        return;
    }
    case OP_ST_Y: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "st(Y)", r);
        st_y(mcu, r);
        return;
    }
    case OP_ST_Y_POSTINC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "st(Y+)", r);
        st_y_postinc(mcu, r);
        return;
    }
    case OP_ST_Y_PREDEC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "st(-Y)", r);
        st_y_predec(mcu, r);
        return;
    }
    case OP_ST_Z: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "st(Z)", r);
        st_z(mcu, r);
        return;
    }
    case OP_ST_Z_POSTINC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "st(Z+)", r);
        st_z_postinc(mcu, r);
        return;
    }
    case OP_ST_Z_PREDEC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "st(-Z)", r);
        st_z_predec(mcu, r);
        return;
    }
    case OP_STS: {
        const u16 k = mcu->flash[mcu->pc + 1];
        const u8 r  = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "sts", r);
        sts(mcu, k, r);
        return;
    }
    case OP_LPM: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "lpm", d);
        lpm(mcu, d);
        return;
    }
    case OP_LPM_POSTINC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "lpm(+)", d);
        lpm_postinc(mcu, d);
        return;
    }
    case OP_PUSH: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "push", r);
        push(mcu, r);
        return;
    }
    case OP_POP: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s r%d", "pop", d);
        pop(mcu, d);
        return;
    }
    }

    /***************************************************************************
     * 8 bit op
     **************************************************************************/
    switch (op & OP_MASK_8) {
    case OP_ADIW: {
        const u8 d = MSH(op, 0x0030, 4);
        const u8 K = MSH(op, 0x00C0, 2) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,%d", "adiw", d, K);
        adiw(mcu, d, K);
        return;
    }
    case OP_SBIW: {
        const u8 d = MSH(op, 0x0030, 4);
        const u8 K = MSH(op, 0x00C0, 2) | MSK(op, 0x000F);
        PRINT_DEBUG("%-6s r%d,%d", "sbiw", d, K);
        sbiw(mcu, d, K);
        return;
    }
    case OP_MULS: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 r = MSK(op, 0x000F) + 16;
        PRINT_DEBUG("%-6s r%d,r%d", "muls", d, r);
        muls(mcu, d, r);
        return;
    }
    case OP_SBIC: {
        const u8 A = MSH(op, 0x00F8, 3);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-6s %d,%d", "sbic", A, b);
        sbic(mcu, A, b);
        return;
    }
    case OP_SBIS: {
        const u8 A = MSH(op, 0x00F8, 3);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-6s %d,%d", "sbis", A, b);
        sbis(mcu, A, b);
        return;
    }
    case OP_SBI: {
        const u8 A = MSH(op, 0x00F8, 3);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-6s %d,%d", "sbi", A, b);
        sbi(mcu, A, b);
        return;
    }
    case OP_CBI: {
        const u8 A = MSH(op, 0x00F8, 3);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-6s %d,%d", "cbi", A, b);
        cbi(mcu, A, b);
        return;
    }
    case OP_MOVW: {
        const u8 d = MSH(op, 0x00F0, 4) * 2;
        const u8 r = MSK(op, 0x000F) * 2;
        PRINT_DEBUG("%-6s r%d,r%d", "movw", d, r);
        movw(mcu, d, r);
        return;
    }
    }

    switch (op & OP_MASK_8_4) {
    case OP_SER: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        PRINT_DEBUG("%-6s r%d", "ser", d);
        ser(mcu, d);
        return;
    }
    }

    /***************************************************************************
     * 9 bit op
     **************************************************************************/
    switch (op & OP_MASK_9_1) {
    case OP_MULSU: {
        const u8 d = MSH(op, 0x0070, 4) + 16;
        const u8 r = MSK(op, 0x0007) + 16;
        PRINT_DEBUG("%-6s r%d,r%d", "mulsu", d, r);
        mulsu(mcu, d, r);
        return;
    }
    case OP_FMUL: {
        const u8 d = MSH(op, 0x0070, 4) + 16;
        const u8 r = MSK(op, 0x0007) + 16;
        PRINT_DEBUG("%-6s r%d,r%d", "fmul", d, r);
        fmul(mcu, d, r);
        return;
    }
    case OP_FMULS: {
        const u8 d = MSH(op, 0x0070, 4) + 16;
        const u8 r = MSK(op, 0x0007) + 16;
        PRINT_DEBUG("%-6s r%d,r%d", "fmuls", d, r);
        fmuls(mcu, d, r);
        return;
    }
    case OP_FMULSU: {
        const u8 d = MSH(op, 0x0070, 4) + 16;
        const u8 r = MSK(op, 0x0007) + 16;
        PRINT_DEBUG("%-6s r%d,r%d", "fmulsu", d, r);
        fmulsu(mcu, d, r);
        return;
    }
    }

    switch (op & OP_MASK_9_4) {
    case OP_BSET: {
        const u8 s = MSH(op, 0x0070, 4);
        PRINT_DEBUG("%-6s %d", "bset", s);
        bset(mcu, s);
        return;
    }
    case OP_BCLR: {
        const u8 s = MSH(op, 0x0070, 4);
        PRINT_DEBUG("%-6s %d", "clr", s);
        bclr(mcu, s);
        return;
    }
    }

    /***************************************************************************
     * 16 bit op
     **************************************************************************/
    switch (op) {
    case OP_IJMP:
        PRINT_DEBUG("%-6s", "ijmp");
        ijmp(mcu);
        return;
    case OP_ICALL:
        PRINT_DEBUG("%-6s", "icall");
        icall(mcu);
        return;
    case OP_RET:
        PRINT_DEBUG("%-6s", "ret");
        ret(mcu);
        return;
    case OP_RETI:
        PRINT_DEBUG("%-6s", "reti");
        reti(mcu);
        return;
    case OP_LPM_R0:
        PRINT_DEBUG("%-6s", "lpm(r0)");
        lpm(mcu, 0);
        return;
    case OP_SPM:
        PRINT_DEBUG("%-6s", "spm");
        spm(mcu);
        return;
    case OP_NOP:
        PRINT_DEBUG("%-6s", "nop");
        nop(mcu);
        return;
    case OP_SLEEP:
        PRINT_DEBUG("%-6s", "sleep");
        sleep(mcu);
        return;
    case OP_WDR:
        PRINT_DEBUG("%-6s", "wdr");
        wdr(mcu);
        return;
    case OP_BREAK:
        PRINT_DEBUG("%-6s", "break");
        break_(mcu);
        return;
    }

    /***************************************************************************
     * Edge case
     **************************************************************************/
    switch (op & OP_MASK_Q) {
    case OP_LDD_Y: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 q = MSH(op, 0x2000, 8) | MSH(op, 0x0C00, 7) | MSK(op, 0x0003);
        PRINT_DEBUG("%-6s r%d,%d", "ldd(Y)", d, q);
        ldd_y(mcu, d, q);
        return;
    }
    case OP_LDD_Z: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 q = MSH(op, 0x2000, 8) | MSH(op, 0x0C00, 7) | MSK(op, 0x0003);
        PRINT_DEBUG("%-6s r%d,%d", "ldd(Z)", d, q);
        ldd_z(mcu, d, q);
        return;
    }
    case OP_STD_Y: {
        const u8 q = MSH(op, 0x2000, 8) | MSH(op, 0x0C00, 7) | MSK(op, 0x0003);
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s %d,r%d", "std(Y)", q, r);
        std_y(mcu, q, r);
        return;
    }
    case OP_STD_Z: {
        const u8 q = MSH(op, 0x2000, 8) | MSH(op, 0x0C00, 7) | MSK(op, 0x0003);
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-6s %d,r%d", "std(Z)", q, r);
        std_z(mcu, q, r);
        return;
    }
    }

    LOG_ERROR("unknown op: %#x pc: %u sp: %u", op, mcu->pc, *mcu->sp);
    exit(EXIT_FAILURE);
}
