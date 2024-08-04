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
#include <stdbool.h>
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
static inline int add(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 1;
}

// adc - add with carry
static inline int adc(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 1;
}

// adiw - add immediate word
static inline int adiw(AVR_MCU *restrict mcu, u8 d, u8 K) {
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

    return 2;
}

// sub - subtract without carry
static inline int sub(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 1;
}

// subi - subtract immediate
static inline int subi(AVR_MCU *restrict mcu, u8 d, u8 K) {
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

    return 1;
}

// sbc - subtract with carry
static inline int sbc(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 1;
}

// sbci - subtract immediate with carry
static inline int sbci(AVR_MCU *restrict mcu, u8 d, u8 K) {
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

    return 1;
}

// sbiw - subtract immediate from word
static inline int sbiw(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 0, 3);
    ASSERT_BOUNDS(K, 0, 63);

    // reg pairs { 24, 26, 28, 30 }
    u16 *Rd = (u16 *)&mcu->reg[d * 2 + 24];

    // R <- Rd - K
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

    return 2;
}

// and - logical and
static inline int and (AVR_MCU *restrict mcu, u8 d, const u8 r) {
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

    return 1;
}

// andi - logical and with immediate
static inline int andi(AVR_MCU *restrict mcu, u8 d, u8 K) {
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

    return 1;
}

// or - logical or
static inline int or (AVR_MCU *restrict mcu, u8 d, const u8 r) {
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

    return 1;
}

// ori - logical or with immediate
static inline int ori(AVR_MCU *restrict mcu, u8 d, u8 K) {
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

    return 1;
}

// eor - exclusive or
static inline int eor(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 1;
}

// com - one's complement
static inline int com(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R = $FF - Rd
    *Rd = 0xFF - *Rd;

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

    return 1;
}

// neg - two's complement
static inline int neg(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R <- $00 - Rd
    const u8 R = 0x00 - *Rd;

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

    return 1;
}

// inc - increment
static inline int inc(AVR_MCU *restrict mcu, u8 d) {
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

    return 1;
}

// dec - decrement
static inline int dec(AVR_MCU *restrict mcu, u8 d) {
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

    return 1;
}

// ser - set all bits in register
static inline int ser(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 16, 31);

    u8 *Rd = &mcu->reg[d];

    // R <- $FF
    *Rd = 0xFF;

    // PC <- PC + 1
    mcu->pc++;

    return 1;
}

// mul - multiply unsigned
static inline int mul(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 2;
}

// muls - multiply signed
static inline int muls(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 2;
}

// mulsu - multiply signed with unsigned
static inline int mulsu(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 2;
}

// fmul - fractional multiply unsigned
static inline int fmul(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 2;
}

// fmuls - fractional multiply signed
static inline int fmuls(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 2;
}

// fmulsu - fractional multiply signed with unsigned
static inline int fmulsu(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 2;
}

/*******************************************************************************
 * Branch Instructions
 ******************************************************************************/

// rjmp - relative jump
static inline int rjmp(AVR_MCU *restrict mcu, i16 k) {
    ASSERT_BOUNDS(k, -2048, 2047);

    // PC <- PC + k + 1
    mcu->pc += k + 1;

    return 2;
}

// ijmp - indirect jump
static inline int ijmp(AVR_MCU *restrict mcu) {
    // PC(15:0) <- Z(15:0)
    mcu->pc = *(u16 *)&mcu->reg[REG_Z];

    return 2;
}

// jmp - jump
static inline int jmp(AVR_MCU *restrict mcu, u16 k) {
    ASSERT_BOUNDS(k, 0, sizeof(mcu->flash) - 1);

    // PC <- k
    mcu->pc = k;

    return 3;
}

// rcall - relative call
static inline int rcall(AVR_MCU *restrict mcu, i16 k) {
    ASSERT_BOUNDS(k, -2048, 2047);

    // SP <- SP - 2
    *mcu->sp -= 2;

    // STACK <- PC + 1
    *(u16 *)&mcu->data[*mcu->sp] = mcu->pc + 1;

    // PC <- PC + k + 1
    mcu->pc += k + 1;

    return 3;
}

// icall indirect call to subroutine
static inline int icall(AVR_MCU *restrict mcu) {
    // SP <- SP - 2
    *mcu->sp -= 2;

    // STACK <- PC + 1
    *(u16 *)&mcu->data[*mcu->sp] = mcu->pc + 1;

    // PC(15:0) <- Z(15:0)
    mcu->pc = *(u16 *)&mcu->reg[REG_Z];

    return 3;
}

// call - int call to a subroutine
static inline int call(AVR_MCU *restrict mcu, u16 k) {
    ASSERT_BOUNDS(k, 0, sizeof(mcu->flash) - 1);

    // SP <- PC - 2
    *mcu->sp -= 2;

    // STACK <- PC + 2
    *(u16 *)&mcu->data[*mcu->sp] = mcu->pc + 2;

    // PC <- k
    mcu->pc = k;

    return 4;
}

// ret - return from subroutine
static inline int ret(AVR_MCU *restrict mcu) {
    // PC(15:0) <- STACK
    mcu->pc = *(u16 *)&mcu->data[*mcu->sp];

    // SP <- PC + 2
    *mcu->sp += 2;

    return 4;
}

// reti - return from interrupt
static inline int reti(AVR_MCU *restrict mcu) {
    // PC(15:0) <- STACK
    mcu->pc = *(u16 *)&mcu->data[*mcu->sp];

    // SP <- PC + 2
    *mcu->sp += 2;

    // I = 1
    PUT_BIT(*mcu->sreg, SREG_I);

    return 4;
}

// cpse - compare skip if equal
static inline int cpse(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rd = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    u8 i = 1;

    // if Rd == Rr then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (*Rd == *Rr) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        i = 2 + IS_32BIT_OP(next_op);
    }

    // PC <- PC + 2 (or 3) if true else PC <- PC + 1
    mcu->pc += i;

    return i;
}

// cp - compare
static inline int cp(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 1;
}

// cpc - compare with carry
static inline int cpc(AVR_MCU *restrict mcu, u8 d, u8 r) {
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

    return 1;
}

// cpi - compare with immediate
static inline int cpi(AVR_MCU *restrict mcu, u8 d, u8 K) {
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

    return 1;
}

// sbrc - skip if bit in register is cleared
static inline int sbrc(AVR_MCU *restrict mcu, u8 r, u8 b) {
    ASSERT_BOUNDS(r, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    const u8 *Rr = &mcu->reg[r];

    u8 i = 1;

    // if Rr(b) = 0 then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (GET_BIT(*Rr, b) == 0) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        i = 2 + IS_32BIT_OP(next_op);
    }

    // PC <- PC + 2 (or 3) if true else PC <- PC + 1
    mcu->pc += i;

    return i;
}

// sbrs - skip if bit in register is set
static inline int sbrs(AVR_MCU *restrict mcu, u8 r, u8 b) {
    ASSERT_BOUNDS(r, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    const u8 *Rr = &mcu->reg[r];

    u8 i = 1;

    // if Rr(b) = 1 then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (GET_BIT(*Rr, b)) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        i = 2 + IS_32BIT_OP(next_op);
    }

    // PC <- PC + 2 (or 3) if true else PC <- PC + 1
    mcu->pc += i;

    return i;
}

// sbic - skip if bit in io register is cleared
static inline int sbic(AVR_MCU *restrict mcu, u8 A, u8 b) {
    ASSERT_BOUNDS(A, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    u8 i = 1;

    // if IO(A,b) = 0 then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (GET_BIT(mcu->io_reg[A], b) == 0) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        i = 2 + IS_32BIT_OP(next_op);
    }

    // PC <- PC + 2 (or 3) if true else PC <- PC + 1
    mcu->pc += i;

    return i;
}

// sbis - skip if bit in io register is set
static inline int sbis(AVR_MCU *restrict mcu, u8 A, u8 b) {
    ASSERT_BOUNDS(A, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    u8 i = 1;

    // if Rr(b) = 1 then PC <- PC + 2 (or 3) else PC <- PC + 1
    if (GET_BIT(mcu->io_reg[A], b)) {
        u16 next_op = mcu->flash[mcu->pc + 1];

        i = 2 + IS_32BIT_OP(next_op);
    }

    // PC <- PC + 2 (or 3) if true else PC <- PC + 1
    mcu->pc += i;

    return i;
}

// brbs - branch if bit in sreg is set
static inline int brbs(AVR_MCU *restrict mcu, u8 s, i8 k) {
    ASSERT_BOUNDS(s, 0, 7);
    ASSERT_BOUNDS(k, -64, 63);

    // PC <- PC + k + 1 if true
    // PC <- PC + 1 if false
    if (GET_BIT(*mcu->sreg, s) == 1) {
        mcu->pc += k + 1;
        return 2;
    } else {
        mcu->pc++;
        return 1;
    }
}

// brbc - branch if bit in sreg is cleared
static inline int brbc(AVR_MCU *restrict mcu, u8 s, i8 k) {
    ASSERT_BOUNDS(s, 0, 7);
    ASSERT_BOUNDS(k, -64, 63);

    // PC <- PC + k + 1 if true
    // PC <- PC + 1 if false
    if (GET_BIT(*mcu->sreg, s) == 0) {
        mcu->pc += k + 1;
        return 2;
    } else {
        mcu->pc++;
        return 1;
    }
}

/*******************************************************************************
 * Bit and Bit-Test Instructions
 ******************************************************************************/

// sbi - set bit in io register
static inline int sbi(AVR_MCU *restrict mcu, u8 A, u8 b) {
    ASSERT_BOUNDS(A, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    // IO(A,b) = 1
    PUT_BIT(mcu->io_reg[A], b);

    // PC <- PC + 1
    mcu->pc++;

    return 2;
}

// cbi - clear bit in io register
static inline int cbi(AVR_MCU *restrict mcu, u8 A, u8 b) {
    ASSERT_BOUNDS(A, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    // IO(A,b) = 0
    CLR_BIT(mcu->io_reg[A], b);

    // PC <- PC + 1
    mcu->pc++;

    return 2;
}

// lsr - logical shift right
static inline int lsr(AVR_MCU *restrict mcu, u8 d) {
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

    return 1;
}

// ror - rotate right through carry
static inline int ror(AVR_MCU *restrict mcu, u8 d) {
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

    return 1;
}

// asr - arithmetic shift right
static inline int asr(AVR_MCU *restrict mcu, u8 d) {
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

    return 1;
}

// swap - swap nibbles
static inline int swap(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R(7:4) = Rd(3:0), R(3:0) = Rd(7:4)
    const u8 R = (*Rd >> 4) | (*Rd << 4);

    // PC <- PC + 1
    mcu->pc++;

    *Rd = R;

    return 1;
}

// bset - bit set in sreg
static inline int bset(AVR_MCU *restrict mcu, u8 s) {
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

    return 1;
}

// bclr - bit clear in sreg
static inline int bclr(AVR_MCU *restrict mcu, u8 s) {
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

    return 1;
}

// bst - bit store from bit in register to T flag in sreg
static inline int bst(AVR_MCU *restrict mcu, u8 r, u8 b) {
    ASSERT_BOUNDS(r, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    const u8 *Rr = &mcu->reg[r];

    // T <- Rr(b)
    SET_BIT(*mcu->sreg, SREG_T, GET_BIT(*Rr, b));

    // PC <- PC + 1
    mcu->pc++;

    // T = 0 if bit b in Rr is cleared. Set to 1 otherwise.

    return 1;
}

// bld - bit load from the T flag in sreg to a bit in register
static inline int bld(AVR_MCU *restrict mcu, u8 d, u8 b) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    u8 *Rd = &mcu->reg[d];

    // Rd(b) <- T
    SET_BIT(*Rd, b, GET_BIT(*mcu->sreg, SREG_T));

    // PC <- PC + 1
    mcu->pc++;

    return 1;
}

/*******************************************************************************
 * Data Transfer Instructions
 ******************************************************************************/

// mov - copy register
static inline int mov(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // Rd <- Rr
    *Rd = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 1;
}

// movw - copy register word
static inline int movw(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 30);
    ASSERT_BOUNDS(r, 0, 30);

    u16 *Rd       = (u16 *)&mcu->reg[d];
    const u16 *Rr = (u16 *)&mcu->reg[r];

    // Rd <- Rr
    *Rd = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 1;
}

// ldi - load immediate
static inline int ldi(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(d, 0, 255);

    u8 *Rd = &mcu->reg[d];

    // Rd <- K
    *Rd = K;

    // PC <- PC + 1
    mcu->pc++;

    return 1;
}

// ld - load indirect from data space to register using index X
static inline int ld_x(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd      = &mcu->reg[d];
    const u16 X = *(u16 *)&mcu->reg[REG_X];

    // Rd <- (X)
    *Rd = mcu->data[X];

    // PC <- PC + 1
    mcu->pc++;

    return 2;
}

static inline int ld_x_postinc(AVR_MCU *restrict mcu, u8 d) {
    ld_x(mcu, d);
    (*(u16 *)&mcu->reg[REG_X])++;

    return 2;
}

static inline int ld_x_predec(AVR_MCU *restrict mcu, u8 d) {
    (*(u16 *)&mcu->reg[REG_X])--;
    ld_x(mcu, d);

    return 2;
}

static inline int ld_y(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd      = &mcu->reg[d];
    const u16 Y = *(u16 *)&mcu->reg[REG_Y];

    // Rd <- (Y)
    *Rd = mcu->data[Y];

    // PC <- PC + 1
    mcu->pc++;

    return 2;
}

static inline int ld_y_postinc(AVR_MCU *restrict mcu, u8 d) {
    ld_y(mcu, d);
    (*(u16 *)&mcu->reg[REG_Y])++;

    return 2;
}

static inline int ld_y_predec(AVR_MCU *restrict mcu, u8 d) {
    (*(u16 *)&mcu->reg[REG_Y])--;
    ld_y(mcu, d);

    return 2;
}

static inline int ld_z(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd      = &mcu->reg[d];
    const u16 Z = *(u16 *)&mcu->reg[REG_Z];

    // Rd <- (Z)
    *Rd = mcu->data[Z];

    // PC <- PC + 1
    mcu->pc++;

    return 2;
}

static inline int ld_z_postinc(AVR_MCU *restrict mcu, u8 d) {
    ld_z(mcu, d);
    (*(u16 *)&mcu->reg[REG_Z])++;

    return 2;
}

static inline int ld_z_predec(AVR_MCU *restrict mcu, u8 d) {
    (*(u16 *)&mcu->reg[REG_Z])--;
    ld_z(mcu, d);

    return 2;
}

// ldd
static inline int ldd_y(AVR_MCU *restrict mcu, u8 d, u8 q) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(q, 0, 63);

    u8 *Rd      = &mcu->reg[d];
    const u16 Y = *(u16 *)&mcu->reg[REG_Y + q];

    // Rd <- (Y)
    *Rd = mcu->data[Y];

    // PC <- PC + 1
    mcu->pc++;

    return 2;
}

static inline int ldd_z(AVR_MCU *restrict mcu, u8 d, u8 q) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(q, 0, 63);

    u8 *Rd      = &mcu->reg[d];
    const u16 Z = *(u16 *)&mcu->reg[REG_Z + q];

    // Rd <- (Z)
    *Rd = mcu->data[Z];

    // PC <- PC + 1
    mcu->pc++;

    return 2;
}

// lds - load direct from data space
static inline int lds(AVR_MCU *restrict mcu, u8 d, u16 k) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(k, 0, sizeof(mcu->data) - 1);

    u8 *Rd = &mcu->reg[d];

    // Rd <- (k)
    *Rd = mcu->data[k];

    // PC <- PC + 2
    mcu->pc += 2;

    return 2;
}

// st - store indirect from register to data space using index X
static inline int st_x(AVR_MCU *restrict mcu, u8 r) {
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 X  = *(u16 *)&mcu->reg[REG_X];

    // (X) <- Rr
    mcu->data[X] = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 1 + IS_IO_SPACE(X);
}

static inline int st_x_postinc(AVR_MCU *restrict mcu, u8 r) {
    int ret = st_x(mcu, r);
    (*(u16 *)&mcu->reg[REG_X])++;
    return ret;
}

static inline int st_x_predec(AVR_MCU *restrict mcu, u8 r) {
    (*(u16 *)&mcu->reg[REG_X])--;
    return st_x(mcu, r) + 1;
}

static inline int st_y(AVR_MCU *restrict mcu, u8 r) {
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 Y  = *(u16 *)&mcu->reg[REG_Y];

    // (Y) <- Rr
    mcu->data[Y] = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 2 + IS_IO_SPACE(Y);
}

static inline int st_y_postinc(AVR_MCU *restrict mcu, u8 r) {
    int ret = st_y(mcu, r);
    (*(u16 *)&mcu->reg[REG_Y])++;

    return ret;
}

static inline int st_y_predec(AVR_MCU *restrict mcu, u8 r) {
    (*(u16 *)&mcu->reg[REG_Y])--;
    return st_y(mcu, r);
}

static inline int st_z(AVR_MCU *restrict mcu, u8 r) {
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 Z  = *(u16 *)&mcu->reg[REG_Z];

    // (Z) <- Rr
    mcu->data[Z] = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 2 + IS_IO_SPACE(Z);
}

static inline int st_z_postinc(AVR_MCU *restrict mcu, u8 r) {
    int ret = st_z(mcu, r);
    (*(u16 *)&mcu->reg[REG_Z])++;

    return ret;
}

static inline int st_z_predec(AVR_MCU *restrict mcu, u8 r) {
    (*(u16 *)&mcu->reg[REG_Z])--;
    return st_z(mcu, r);
}

// std
static inline int std_y(AVR_MCU *restrict mcu, u8 q, u8 r) {
    ASSERT_BOUNDS(q, 0, 63);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 Y  = *(u16 *)&mcu->reg[REG_Y + q];

    // (Y) <- Rr
    mcu->data[Y] = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 2 + IS_IO_SPACE(Y);
}

static inline int std_z(AVR_MCU *restrict mcu, u8 q, u8 r) {
    ASSERT_BOUNDS(q, 0, 63);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];
    const u16 Z  = *(u16 *)&mcu->reg[REG_Z + q];

    // (Z) <- Rr
    mcu->data[Z] = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 2 + IS_IO_SPACE(Z);
}

// sts - store direct to data space
static inline int sts(AVR_MCU *restrict mcu, u16 k, u8 r) {
    ASSERT_BOUNDS(k, 0, sizeof(mcu->data) - 1);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];

    // (k) <- Rr
    mcu->data[k] = *Rr;

    // PC <- PC + 2
    mcu->pc += 2;

    return 2 + IS_IO_SPACE(k);
}

// lpm - load program memory
static inline int lpm(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd      = &mcu->reg[d];
    const u16 Z = *(u16 *)&mcu->reg[REG_Z];

    // Rd <- (Z)
    *Rd = ((u8 *)&mcu->flash)[Z];

    // PC <- PC + 1
    mcu->pc += 1;

    return 3;
}

static inline int lpm_postinc(AVR_MCU *restrict mcu, u8 d) {
    lpm(mcu, d);
    (*(u16 *)&mcu->reg[REG_Z])++;

    return 3;
}

// spm - store program memory
static inline int spm(AVR_MCU *restrict mcu) {
    const u16 *Rr = (u16 *)&mcu->reg[0];
    const u16 Z   = *(u16 *)&mcu->reg[REG_Z];

    // (Z) <- Rr
    mcu->flash[Z] = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 0; // special case, used from EEPROM writing. Will be int time (TODO)
}

// in - load an io location to register
static inline int in(AVR_MCU *restrict mcu, u8 d, u8 A) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(A, 0, 63);

    u8 *Rd = &mcu->reg[d];

    // Rd <- IO(A)
    *Rd = mcu->io_reg[A];

    // PC <- PC + 1
    mcu->pc++;

    return 1;
}

// out - store register to io location
static inline int out(AVR_MCU *restrict mcu, u8 A, u8 r) {
    ASSERT_BOUNDS(A, 0, 63);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];

    // IO(A) <- Rr
    mcu->io_reg[A] = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 1;
}

// push - push register on stack
static inline int push(AVR_MCU *restrict mcu, u8 r) {
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rr = &mcu->reg[r];

    // SP <- SP - 1
    (*mcu->sp)--;

    // STACK <- Rr
    mcu->data[*mcu->sp] = *Rr;

    // PC <- PC + 1
    mcu->pc++;

    return 2;
}

// pop - pop register from stack
static inline int pop(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // Rd <- STACK
    *Rd = mcu->data[*mcu->sp];

    // SP <- SP + 1
    (*mcu->sp)++;

    // PC <- PC + 1
    mcu->pc++;

    return 2;
}

/*******************************************************************************
 * MCU Control Instructions
 ******************************************************************************/

// nop - no operation
static inline int nop(AVR_MCU *restrict mcu) {
    // PC <- PC + 1
    mcu->pc++;

    return 1;
}

// sleep -
static inline int sleep(AVR_MCU *restrict mcu) {
    switch (mcu->data[REG_SMCR]) {
    case SLEEP_IDLE:
        mcu->idle = true;
        break;
    case SLEEP_ADC_NR:
    case SLEEP_POWER_DOWN:
    case SLEEP_POWER_SAVE:
        LOG_DEBUG("SLEEP MODE %#x TODO", mcu->data[REG_SMCR]);
        break;
    }

    // PC <- PC + 1
    mcu->pc++;

    return 1;
}

// wdr - watchdog reset
static inline int wdr(AVR_MCU *restrict mcu) {
    return nop(mcu);
}

// break - break
static inline int break_(AVR_MCU *restrict mcu) {
    return nop(mcu);
}

static inline u8 xstr2byte(const char *restrict s) {
    u8 b = 0;
    b |= (isdigit(s[0]) ? s[0] - '0' : isupper(s[0]) ? s[0] - 'A' + 10 : s[0] - 'a' + 10) << 4;
    b |= (isdigit(s[1]) ? s[1] - '0' : isupper(s[1]) ? s[1] - 'A' + 10 : s[1] - 'a' + 10);
    return b;
}

// get clock prescaler
// if returns 0 that means clock is off
static inline u16 get_clk_ps(u8 bitfield) {
    switch (bitfield) {
    case 1:
        return 1;
    case 2:
        return 8;
    case 3:
        return 64;
    case 4:
        return 256;
    case 5:
        return 1024;
    default:
        return 0;
    };
}

// compare output used for setting OCx pins
// NOLINTNEXTLINE
static inline void comp_normal(AVR_MCU *restrict mcu, u8 reg, u8 bit, u8 com) {
    switch (com) {
    case 1:
        TGL_BIT(mcu->data[reg], bit);
        break;
    case 2:
        CLR_BIT(mcu->data[reg], bit);
        break;
    case 3:
        PUT_BIT(mcu->data[reg], bit);
        break;
    }
}

// compare output used for setting OCx pins in PWM mode
// for fase PWM reverse is set when at BOTTOM
// for phase correct PWM reverse is set when counting DOWN
// NOTE WGM02 must equal 1
// NOLINTNEXTLINE
static inline void comp_pwm(AVR_MCU *restrict mcu, u8 reg, u8 bit, u8 com, bool reverse) {
    switch (com) {
    case 1:
        TGL_BIT(mcu->data[reg], bit);
        break;
    case 2:
        SET_BIT(mcu->data[reg], bit, reverse);
        break;
    case 3:
        SET_BIT(mcu->data[reg], bit, !reverse);
        break;
    }
}

// need a function per timer because there are slight variations in each
static inline void timer0_tick(AVR_MCU *restrict mcu) {
    // clk divisor
    const u16 div0 = get_clk_ps(mcu->data[REG_TCCR0B] & 0x07);
    if (div0 == 0 || mcu->clk % div0) {
        return;
    }

    u8 *const tcnt0 = &mcu->data[REG_TCNT0];
    const u8 wgm0   = MSH(mcu->data[REG_TCCR0B], 0x08, 1) | MSK(mcu->data[REG_TCCR0A], 0x03);
    const u8 coma0  = MSH(mcu->data[REG_TCCR0A], 0xC0, 6);
    const u8 comb0  = MSH(mcu->data[REG_TCCR0A], 0x30, 4);
    u16 top0        = 0xFF;

    switch (wgm0) {
    case 0: // NORMAL
        (*tcnt0)++;

        if (*tcnt0 == mcu->data[REG_OCR0A]) {
            PUT_BIT(mcu->data[REG_TIFR0], BIT_OCF0A);
            comp_normal(mcu, REG_PORTD, 6, coma0);
        }
        if (*tcnt0 == mcu->data[REG_OCR0B]) {
            PUT_BIT(mcu->data[REG_TIFR0], BIT_OCF0B);
            comp_normal(mcu, REG_PORTD, 5, comb0);
        }
        break;
    case 2: // CTC
        top0   = mcu->data[REG_OCR0A];
        *tcnt0 = (*tcnt0 + 1) % (top0 + 1);

        if (*tcnt0 == mcu->data[REG_OCR0A]) {
            PUT_BIT(mcu->data[REG_TIFR0], BIT_OCF0A);
            comp_normal(mcu, REG_PORTD, 6, coma0);
        }
        if (*tcnt0 == mcu->data[REG_OCR0B]) {
            PUT_BIT(mcu->data[REG_TIFR0], BIT_OCF0B);
            comp_normal(mcu, REG_PORTD, 5, comb0);
        }
        break;
    case 7: // Fast PWM Mode
        top0 = mcu->data[REG_OCR0A];
        __attribute__((fallthrough));
    case 3:

        *tcnt0 = (*tcnt0 + 1) % (top0 + 1);

        if (*tcnt0 == mcu->data[REG_OCR0A]) {
            PUT_BIT(mcu->data[REG_TIFR0], BIT_OCF0A);
            comp_pwm(mcu, REG_PORTD, 6, coma0, false);
        }
        if (*tcnt0 == mcu->data[REG_OCR0B]) {
            PUT_BIT(mcu->data[REG_TIFR0], BIT_OCF0B);
            comp_pwm(mcu, REG_PORTD, 5, comb0, false);
        }
        if (*tcnt0 == 0) {
            if (GET_BIT(coma0, 2))
                comp_pwm(mcu, REG_PORTD, 6, coma0, true);
            if (GET_BIT(comb0, 2))
                comp_pwm(mcu, REG_PORTD, 5, comb0, true);
        }
        break;
    case 5: // Phase Correct PWM Mode
        top0 = mcu->data[REG_OCR0A];
        __attribute__((fallthrough));
    case 1:
        *tcnt0 = mcu->pwm_invert ? -1 : 1;

        if (*tcnt0 == mcu->data[REG_OCR0A]) {
            PUT_BIT(mcu->data[REG_TIFR0], BIT_OCF0A);
            comp_pwm(mcu, REG_PORTD, 6, coma0, mcu->pwm_invert);
        }
        if (*tcnt0 == mcu->data[REG_OCR0B]) {
            PUT_BIT(mcu->data[REG_TIFR0], BIT_OCF0B);
            comp_pwm(mcu, REG_PORTD, 5, comb0, mcu->pwm_invert);
        }
        if (*tcnt0 == 0 || *tcnt0 == top0) {
            mcu->pwm_invert = !mcu->pwm_invert;
        }
        break;
    default:
        LOG_ERROR("unknown waveform generator mode");
        return;
    }

    mcu->data[REG_TIFR0] |= (*tcnt0 == 0); // only set never clear
}

// need a function per timer because there are slight variations in each
static inline void timer1_tick(AVR_MCU *restrict mcu) {
    // clk divisor
    const u16 div1 = get_clk_ps(mcu->data[REG_TCCR1B] & 0x07);
    if (div1 == 0 || mcu->clk % div1) {
        return;
    }

    u16 *const tcnt1 = (u16 *)&mcu->data[REG_TCNT1L];
    const u8 wgm1    = MSH(mcu->data[REG_TCCR1B], 0x18, 3) | MSK(mcu->data[REG_TCCR1A], 0x03);
    const u8 coma1   = MSH(mcu->data[REG_TCCR1A], 0xC0, 6);
    const u8 comb1   = MSH(mcu->data[REG_TCCR1A], 0x30, 4);
    u16 top1;

    switch (wgm1) {
    case 1:
    case 5:
        top1 = 0x00FF;
        break;
    case 2:
    case 6:
        top1 = 0x01FF;
        break;
    case 3:
    case 7:
        top1 = 0x03FF;
        break;
    case 10:
    case 14:
        top1 = *(u16 *)&mcu->data[REG_ICR1L];
        break;
    case 11:
    case 15:
        top1 = *(u16 *)&mcu->data[REG_OCR1AL];
        break;
    }

    switch (wgm1) {
    case 0: // NORMAL
        (*tcnt1)++;

        if (*tcnt1 == *(u16 *)&mcu->data[REG_OCR1AL]) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1A);
            comp_normal(mcu, REG_PORTB, 1, coma1);
        }
        if (*tcnt1 == *(u16 *)&mcu->data[REG_OCR1BL]) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1B);
            comp_normal(mcu, REG_PORTB, 2, comb1);
        }
        break;
    case 4: // CTC
        top1   = *(u16 *)&mcu->data[REG_OCR1AL];
        *tcnt1 = (*tcnt1 + 1) % (top1 + 1);

        if (*tcnt1 == *(u16 *)&mcu->data[REG_OCR1AL]) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1A);
            comp_normal(mcu, REG_PORTB, 1, coma1);
        }
        if (*tcnt1 == *(u16 *)&mcu->data[REG_OCR1BL]) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1B);
            comp_normal(mcu, REG_PORTB, 2, comb1);
        }
        break;
    case 12: // CTC
        top1   = *(u16 *)&mcu->data[REG_ICR1L];
        *tcnt1 = (*tcnt1 + 1) % (top1 + 1);

        if (wgm1 == 12 && *tcnt1 == *(u16 *)&mcu->data[REG_ICR1L]) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1A);
            comp_normal(mcu, REG_PORTB, 1, coma1);
        }
        if (*tcnt1 == *(u16 *)&mcu->data[REG_OCR1BL]) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1B);
            comp_normal(mcu, REG_PORTB, 2, comb1);
        }
        break;
    case 5: // Fast PWM Mode
    case 6:
    case 7:
    case 14:
    case 15:
        *tcnt1 = (*tcnt1 + 1) % (top1 + 1);

        if ((wgm1 == 14 && *tcnt1 == *(u16 *)&mcu->data[REG_ICR1L]) ||
            (wgm1 != 14 && *tcnt1 == *(u16 *)&mcu->data[REG_OCR1AL])) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1A);
            comp_pwm(mcu, REG_PORTB, 1, coma1, false);
        }
        if (*tcnt1 == *(u16 *)&mcu->data[REG_OCR1BL]) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1B);
            comp_pwm(mcu, REG_PORTB, 2, comb1, false);
        }
        if (*tcnt1 == 0) {
            if (GET_BIT(coma1, 2))
                comp_pwm(mcu, REG_PORTB, 1, coma1, true);
            if (GET_BIT(comb1, 2))
                comp_pwm(mcu, REG_PORTB, 2, comb1, true);
        }
        break;
    case 1:
    case 2:
    case 3:
    case 8:
    case 9:
        *tcnt1 = mcu->pwm_invert ? -1 : 1;

        if ((wgm1 == 14 && *tcnt1 == *(u16 *)&mcu->data[REG_ICR1L]) ||
            (wgm1 != 14 && *tcnt1 == *(u16 *)&mcu->data[REG_OCR1AL])) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1A);
            comp_pwm(mcu, REG_PORTB, 1, coma1, mcu->pwm_invert);
        }
        if (*tcnt1 == *(u16 *)&mcu->data[REG_OCR1BL]) {
            PUT_BIT(mcu->data[REG_TIFR1], BIT_OCF1B);
            comp_pwm(mcu, REG_PORTB, 2, comb1, mcu->pwm_invert);
        }
        if (*tcnt1 == 0 || *tcnt1 == top1) {
            mcu->pwm_invert = !mcu->pwm_invert;
        }
        break;
    default:
        LOG_ERROR("unknown waveform generator mode");
        return;
    }

    mcu->data[REG_TIFR1] |= (*tcnt1 == 0); // only set never clear
}

static inline void timer2_tick(AVR_MCU *restrict mcu) {
    // clk divisor
    const u16 div2 = get_clk_ps(mcu->data[REG_TCCR2B] & 0x07);
    if (div2 == 0 || mcu->clk % div2) {
        return;
    }

    u8 *const tcnt2 = &mcu->data[REG_TCNT2];
    const u8 wgm2   = MSH(mcu->data[REG_TCCR2B], 0x08, 1) | MSK(mcu->data[REG_TCCR2A], 0x03);
    const u8 coma2  = MSH(mcu->data[REG_TCCR2A], 0xC0, 6);
    const u8 comb2  = MSH(mcu->data[REG_TCCR2A], 0x30, 4);
    u16 top2        = 0xFF;

    switch (wgm2) {
    case 0: // NORMAL
        (*tcnt2)++;

        if (*tcnt2 == mcu->data[REG_OCR2A]) {
            PUT_BIT(mcu->data[REG_TIFR2], BIT_OCF2A);
            comp_normal(mcu, REG_PORTB, 3, coma2);
        }
        if (*tcnt2 == mcu->data[REG_OCR2B]) {
            PUT_BIT(mcu->data[REG_TIFR2], BIT_OCF2B);
            comp_normal(mcu, REG_PORTD, 3, comb2);
        }
        break;
    case 2: // CTC
        top2   = mcu->data[REG_OCR2A];
        *tcnt2 = (*tcnt2 + 1) % (top2 + 1);

        if (*tcnt2 == mcu->data[REG_OCR2A]) {
            PUT_BIT(mcu->data[REG_TIFR2], BIT_OCF2A);
            comp_normal(mcu, REG_PORTB, 3, coma2);
        }
        if (*tcnt2 == mcu->data[REG_OCR2B]) {
            PUT_BIT(mcu->data[REG_TIFR2], BIT_OCF2B);
            comp_normal(mcu, REG_PORTD, 3, comb2);
        }
        break;
    case 7: // Fast PWM Mode
        top2 = mcu->data[REG_OCR2A];
        __attribute__((fallthrough));
    case 3:
        *tcnt2 = (*tcnt2 + 1) % (top2 + 1);

        if (*tcnt2 == mcu->data[REG_OCR2A]) {
            PUT_BIT(mcu->data[REG_TIFR2], BIT_OCF2A);
            comp_pwm(mcu, REG_PORTB, 3, coma2, false);
        }
        if (*tcnt2 == mcu->data[REG_OCR2B]) {
            PUT_BIT(mcu->data[REG_TIFR2], BIT_OCF2B);
            comp_pwm(mcu, REG_PORTD, 3, comb2, false);
        }
        if (*tcnt2 == 0) {
            if (GET_BIT(coma2, 2))
                comp_pwm(mcu, REG_PORTB, 3, coma2, true);
            if (GET_BIT(comb2, 2))
                comp_pwm(mcu, REG_PORTD, 3, comb2, true);
        }
        break;
    case 5: // Phase Correct PWM Mode
        top2 = mcu->data[REG_OCR2A];
        __attribute__((fallthrough));
    case 1:
        *tcnt2 = mcu->pwm_invert ? -1 : 1;

        if (*tcnt2 == mcu->data[REG_OCR2A]) {
            PUT_BIT(mcu->data[REG_TIFR2], BIT_OCF2A);
            comp_pwm(mcu, REG_PORTB, 3, coma2, mcu->pwm_invert);
        }
        if (*tcnt2 == mcu->data[REG_OCR2B]) {
            PUT_BIT(mcu->data[REG_TIFR2], BIT_OCF2B);
            comp_pwm(mcu, REG_PORTD, 3, comb2, mcu->pwm_invert);
        }
        if (*tcnt2 == 0 || *tcnt2 == top2) {
            mcu->pwm_invert = !mcu->pwm_invert;
        }
        break;
    default:
        LOG_ERROR("unknown waveform generator mode");
        return;
    }

    mcu->data[REG_TIFR2] |= (*tcnt2 == 0); // only set never clear
}

// enter an interrupt service routine
// this should be called after an execute call so we store current pc
// takes 4 cycles just like a normal call instruction
// iv : interrupt vector
static inline int isr(AVR_MCU *restrict mcu, u16 iv) {
    ASSERT_BOUNDS(iv, 0, sizeof(mcu->flash) - 1);

    // SP <- PC - 2
    *mcu->sp -= 2;

    // STACK <- PC
    *(u16 *)&mcu->data[*mcu->sp] = mcu->pc;

    // PC <- iv
    mcu->pc = iv;

    return 4;
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

    // initial values
    mcu->data[REG_UCSR0A] |= 0x20; // 0010 0000
    mcu->data[REG_UCSR0C] |= 0x06; // 0000 0110
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

int avr_execute(AVR_MCU *const restrict mcu) {
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
        PRINT_DEBUG("%-8s r%-7d %-8d", "subi", d, K);
        return subi(mcu, d, K);
    }
    case OP_SBCI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d %-8d", "sbci", d, K);
        return sbci(mcu, d, K);
    }
    case OP_ANDI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d %-8d", "andi", d, K);
        return andi(mcu, d, K);
    }
    case OP_ORI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d %-8d", "ori", d, K);
        return ori(mcu, d, K);
    }
    case OP_RJMP: {
        const i16 k = I12_TO_I16(MSK(op, 0x0FFF));
        PRINT_DEBUG("%-8s %-17d", "rjmp", k);
        return rjmp(mcu, k);
    }
    case OP_RCALL: {
        const i16 k = I12_TO_I16(MSK(op, 0x0FFF));
        PRINT_DEBUG("%-8s %-17d", "rcall", k);
        return rcall(mcu, k);
    }
    case OP_CPI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d %-8d", "cpi", d, K);
        return cpi(mcu, d, K);
    }
    case OP_LDI: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 K = MSH(op, 0x0F00, 4) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d %-8d", "ldi", d, K);
        return ldi(mcu, d, K);
    }
    }

    /***************************************************************************
     * 5 bit op
     **************************************************************************/
    switch (op & OP_MASK_5) {
    case OP_IN: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 A = MSH(op, 0x0600, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d %-8d", "in", d, A);
        return in(mcu, d, A);
    }
    case OP_OUT: {
        const u8 A = MSH(op, 0x0600, 5) | MSK(op, 0x000F);
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s %-8d r%-7d", "out", A, r);
        return out(mcu, A, r);
    }
    }

    /***************************************************************************
     * 6 bit op
     **************************************************************************/
    switch (op & OP_MASK_6) {
    case OP_ADD: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "add", d, r);
        return add(mcu, d, r);
    }
    case OP_ADC: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "adc", d, r);
        return adc(mcu, d, r);
    }
    case OP_SUB: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "sub", d, r);
        return sub(mcu, d, r);
    }
    case OP_SBC: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "sbc", d, r);
        return sbc(mcu, d, r);
    }
    case OP_AND: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "and", d, r);
        return and(mcu, d, r);
    }
    case OP_OR: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "or", d, r);
        return or (mcu, d, r);
    }
    case OP_EOR: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "eor", d, r);
        return eor(mcu, d, r);
    }
    case OP_MUL: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "mul", d, r);
        return mul(mcu, d, r);
    }
    case OP_CPSE: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "cpse", d, r);
        return cpse(mcu, d, r);
    }
    case OP_CP: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "cp", d, r);
        return cp(mcu, d, r);
    }
    case OP_CPC: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d r%-7d", "cpc", d, r);
        return cpc(mcu, d, r);
    }
    case OP_BRBC: {
        const u8 s = MSK(op, 0x0003);
        const i8 k = I7_TO_I16(MSH(op, 0x03F8, 3));
        PRINT_DEBUG("%-8s %-8d %-8d", "brbc", s, k);
        return brbc(mcu, s, k);
    }
    case OP_BRBS: {
        const u8 s = MSK(op, 0x0003);
        const i8 k = I7_TO_I16(MSH(op, 0x03F8, 3));
        PRINT_DEBUG("%-8s %-8d %-8d", "brbs", s, k);
        return brbs(mcu, s, k);
    }
    case OP_MOV: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 r = MSH(op, 0x0200, 5) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d ,r%-7d", "mov", d, r);
        return mov(mcu, d, r);
    }
    }

    /***************************************************************************
     * 7 bit op
     **************************************************************************/
    switch (op & OP_MASK_7_1) {
    case OP_SBRC: {
        const u8 r = MSH(op, 0x01F0, 4);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-8s r%-7d %-8d", "sbrc", r, b);
        return sbrc(mcu, r, b);
    }
    case OP_SBRS: {
        const u8 r = MSH(op, 0x01F0, 4);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-8s r%-7d %-8d", "sbrs", r, b);
        return sbrs(mcu, r, b);
    }
    case OP_BST: {
        const u8 r = MSH(op, 0x01F0, 4);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-8s r%-7d %-8d", "bst", r, b);
        return bst(mcu, r, b);
    }
    case OP_BLD: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-8s r%-7d %-8d", "bld", d, b);
        return bld(mcu, d, b);
    }
    }

    switch (op & OP_MASK_7_3) {
    case OP_JMP: {
        const u16 k = mcu->flash[mcu->pc + 1]; // works because address space fits in 16bits
        PRINT_DEBUG("%-8s %-17d", "jmp", k);
        return jmp(mcu, k);
    }
    case OP_CALL: {
        const u16 k = mcu->flash[mcu->pc + 1]; // works because address space fits in 16bits
        PRINT_DEBUG("%-8s %-17d", "call", k);
        return call(mcu, k);
    }
    }

    switch (op & OP_MASK_7_4) {
    case OP_COM: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "com", d);
        return com(mcu, d);
    }
    case OP_NEG: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "neg", d);
        return neg(mcu, d);
    }
    case OP_INC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "inc", d);
        return inc(mcu, d);
    }
    case OP_DEC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "dec", d);
        return dec(mcu, d);
    }
    case OP_LSR: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "lsr", d);
        return lsr(mcu, d);
    }
    case OP_ROR: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ror", d);
        return ror(mcu, d);
    }
    case OP_ASR: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "asr", d);
        return asr(mcu, d);
    }
    case OP_SWAP: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "swap", d);
        return swap(mcu, d);
    }
    case OP_LD_X: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ld(X)", d);
        return ld_x(mcu, d);
    }
    case OP_LD_X_POSTINC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ld(X+)", d);
        return ld_x_postinc(mcu, d);
    }
    case OP_LD_X_PREDEC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ld(-X)", d);
        return ld_x_predec(mcu, d);
    }
    case OP_LD_Y: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ld(Y)", d);
        return ld_y(mcu, d);
    }
    case OP_LD_Y_POSTINC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ld(Y+)", d);
        return ld_y_postinc(mcu, d);
    }
    case OP_LD_Y_PREDEC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ld(-Y)", d);
        return ld_y_predec(mcu, d);
    }
    case OP_LD_Z: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ld(Z)", d);
        return ld_z(mcu, d);
    }
    case OP_LD_Z_POSTINC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ld(Z+)", d);
        return ld_z_postinc(mcu, d);
    }
    case OP_LD_Z_PREDEC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "ld(-Z)", d);
        return ld_z_predec(mcu, d);
    }
    case OP_LDS: {
        const u8 d  = MSH(op, 0x01F0, 4);
        const u16 k = mcu->flash[mcu->pc + 1];
        PRINT_DEBUG("%-8s r%-16d", "lds", d);
        return lds(mcu, d, k);
    }
    case OP_ST_X: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "st(X)", r);
        return st_x(mcu, r);
    }
    case OP_ST_X_POSTINC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "st(X+)", r);
        return st_x_postinc(mcu, r);
    }
    case OP_ST_X_PREDEC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "st(-X)", r);
        return st_x_predec(mcu, r);
    }
    case OP_ST_Y: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "st(Y)", r);
        return st_y(mcu, r);
    }
    case OP_ST_Y_POSTINC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "st(Y+)", r);
        return st_y_postinc(mcu, r);
    }
    case OP_ST_Y_PREDEC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "st(-Y)", r);
        return st_y_predec(mcu, r);
    }
    case OP_ST_Z: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "st(Z)", r);
        return st_z(mcu, r);
    }
    case OP_ST_Z_POSTINC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "st(Z+)", r);
        return st_z_postinc(mcu, r);
    }
    case OP_ST_Z_PREDEC: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "st(-Z)", r);
        return st_z_predec(mcu, r);
    }
    case OP_STS: {
        const u16 k = mcu->flash[mcu->pc + 1];
        const u8 r  = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "sts", r);
        return sts(mcu, k, r);
    }
    case OP_LPM: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "lpm", d);
        return lpm(mcu, d);
    }
    case OP_LPM_POSTINC: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "lpm(+)", d);
        return lpm_postinc(mcu, d);
    }
    case OP_PUSH: {
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "push", r);
        return push(mcu, r);
    }
    case OP_POP: {
        const u8 d = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s r%-16d", "pop", d);
        return pop(mcu, d);
    }
    }

    /***************************************************************************
     * 8 bit op
     **************************************************************************/
    switch (op & OP_MASK_8) {
    case OP_ADIW: {
        const u8 d = MSH(op, 0x0030, 4);
        const u8 K = MSH(op, 0x00C0, 2) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d %-8d", "adiw", d, K);
        return adiw(mcu, d, K);
    }
    case OP_SBIW: {
        const u8 d = MSH(op, 0x0030, 4);
        const u8 K = MSH(op, 0x00C0, 2) | MSK(op, 0x000F);
        PRINT_DEBUG("%-8s r%-7d %-8d", "sbiw", d, K);
        return sbiw(mcu, d, K);
    }
    case OP_MULS: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        const u8 r = MSK(op, 0x000F) + 16;
        PRINT_DEBUG("%-8s r%-7d r%-7d", "muls", d, r);
        return muls(mcu, d, r);
    }
    case OP_SBIC: {
        const u8 A = MSH(op, 0x00F8, 3);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-8s %-8d %-8d", "sbic", A, b);
        return sbic(mcu, A, b);
    }
    case OP_SBIS: {
        const u8 A = MSH(op, 0x00F8, 3);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-8s %-8d %-8d", "sbis", A, b);
        return sbis(mcu, A, b);
    }
    case OP_SBI: {
        const u8 A = MSH(op, 0x00F8, 3);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-8s %-8d %-8d", "sbi", A, b);
        return sbi(mcu, A, b);
    }
    case OP_CBI: {
        const u8 A = MSH(op, 0x00F8, 3);
        const u8 b = MSK(op, 0x0003);
        PRINT_DEBUG("%-8s %-8d %-8d", "cbi", A, b);
        return cbi(mcu, A, b);
    }
    case OP_MOVW: {
        const u8 d = MSH(op, 0x00F0, 4) * 2;
        const u8 r = MSK(op, 0x000F) * 2;
        PRINT_DEBUG("%-8s r%-7d r%-7d", "movw", d, r);
        return movw(mcu, d, r);
    }
    }

    switch (op & OP_MASK_8_4) {
    case OP_SER: {
        const u8 d = MSH(op, 0x00F0, 4) + 16;
        PRINT_DEBUG("%-8s r%-7d", "ser", d);
        return ser(mcu, d);
    }
    }

    /***************************************************************************
     * 9 bit op
     **************************************************************************/
    switch (op & OP_MASK_9_1) {
    case OP_MULSU: {
        const u8 d = MSH(op, 0x0070, 4) + 16;
        const u8 r = MSK(op, 0x0007) + 16;
        PRINT_DEBUG("%-8s r%-7d r%-7d", "mulsu", d, r);
        return mulsu(mcu, d, r);
    }
    case OP_FMUL: {
        const u8 d = MSH(op, 0x0070, 4) + 16;
        const u8 r = MSK(op, 0x0007) + 16;
        PRINT_DEBUG("%-8s r%-7d r%-7d", "fmul", d, r);
        return fmul(mcu, d, r);
    }
    case OP_FMULS: {
        const u8 d = MSH(op, 0x0070, 4) + 16;
        const u8 r = MSK(op, 0x0007) + 16;
        PRINT_DEBUG("%-8s r%-7d r%-7d", "fmuls", d, r);
        return fmuls(mcu, d, r);
    }
    case OP_FMULSU: {
        const u8 d = MSH(op, 0x0070, 4) + 16;
        const u8 r = MSK(op, 0x0007) + 16;
        PRINT_DEBUG("%-8s r%-7d r%-7d", "fmulsu", d, r);
        return fmulsu(mcu, d, r);
    }
    }

    switch (op & OP_MASK_9_4) {
    case OP_BSET: {
        const u8 s = MSH(op, 0x0070, 4);
        PRINT_DEBUG("%-8s %-17d", "bset", s);
        return bset(mcu, s);
    }
    case OP_BCLR: {
        const u8 s = MSH(op, 0x0070, 4);
        PRINT_DEBUG("%-8s %-17d", "clr", s);
        return bclr(mcu, s);
    }
    }

    /***************************************************************************
     * 16 bit op
     **************************************************************************/
    switch (op) {
    case OP_IJMP:
        PRINT_DEBUG("%-26s", "ijmp");
        return ijmp(mcu);
    case OP_ICALL:
        PRINT_DEBUG("%-26s", "icall");
        return icall(mcu);
    case OP_RET:
        PRINT_DEBUG("%-26s", "ret");
        return ret(mcu);
    case OP_RETI:
        PRINT_DEBUG("%-26s", "reti");
        return reti(mcu);
    case OP_LPM_R0:
        PRINT_DEBUG("%-26s", "lpm(r0)");
        return lpm(mcu, 0);
    case OP_SPM:
        PRINT_DEBUG("%-26s", "spm");
        return spm(mcu);
    case OP_NOP:
        PRINT_DEBUG("%-26s", "nop");
        return nop(mcu);
    case OP_SLEEP:
        PRINT_DEBUG("%-26s", "sleep");
        return sleep(mcu);
    case OP_WDR:
        PRINT_DEBUG("%-26s", "wdr");
        return wdr(mcu);
    case OP_BREAK:
        PRINT_DEBUG("%-26s", "break");
        return break_(mcu);
    }

    /***************************************************************************
     * Edge case
     **************************************************************************/
    switch (op & OP_MASK_Q) {
    case OP_LDD_Y: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 q = MSH(op, 0x2000, 8) | MSH(op, 0x0C00, 7) | MSK(op, 0x0003);
        PRINT_DEBUG("%-8s r%-7d %-8d", "ldd(Y)", d, q);
        return ldd_y(mcu, d, q);
    }
    case OP_LDD_Z: {
        const u8 d = MSH(op, 0x01F0, 4);
        const u8 q = MSH(op, 0x2000, 8) | MSH(op, 0x0C00, 7) | MSK(op, 0x0003);
        PRINT_DEBUG("%-8s r%-7d %-8d", "ldd(Z)", d, q);
        return ldd_z(mcu, d, q);
    }
    case OP_STD_Y: {
        const u8 q = MSH(op, 0x2000, 8) | MSH(op, 0x0C00, 7) | MSK(op, 0x0003);
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s %-8d r%-7d", "std(Y)", q, r);
        return std_y(mcu, q, r);
    }
    case OP_STD_Z: {
        const u8 q = MSH(op, 0x2000, 8) | MSH(op, 0x0C00, 7) | MSK(op, 0x0003);
        const u8 r = MSH(op, 0x01F0, 4);
        PRINT_DEBUG("%-8s %-8d r%-7d", "std(Z)", q, r);
        return std_z(mcu, q, r);
    }
    }

    LOG_ERROR("unknown op: %#x pc: %lu sp: %u", op, mcu->pc, *mcu->sp);
    exit(EXIT_FAILURE);
}

int avr_interrupt(AVR_MCU *restrict mcu) {
    // global interrupts are disabled
    if (GET_BIT(*mcu->sreg, SREG_I) == 0) {
        return 0;
    }

    // clear global interrupt before calling one
    CLR_BIT(*mcu->sreg, SREG_I);

    // reset
    if (mcu->data[REG_MCUSR]) {
        mcu->data[REG_MCUSR] = 0;
        PRINT_DEBUG("int reset");
        return isr(mcu, IV_RESET);
    }

    // int0
    // int1
    // pcint0
    // pcint1
    // pcint2

    // wdt (UNUSED)

    if (mcu->data[REG_TIMSK2]) {
        // timer2 compa
        if (GET_BIT(mcu->data[REG_TIFR2], BIT_OCF2A) && GET_BIT(mcu->data[REG_TIMSK2], 1)) {
            CLR_BIT(mcu->data[REG_TIFR2], BIT_OCF2A);
            PRINT_DEBUG("int timer2 compa");
            return isr(mcu, IV_TIMER2_COMPA);
        }
        // timer2 compb
        if (GET_BIT(mcu->data[REG_TIFR2], BIT_OCF2B) && GET_BIT(mcu->data[REG_TIMSK2], 2)) {
            CLR_BIT(mcu->data[REG_TIFR2], BIT_OCF2B);
            PRINT_DEBUG("int timer2 compb");
            return isr(mcu, IV_TIMER2_COMPB);
        }
        // timer2 ovf
        if (GET_BIT(mcu->data[REG_TIFR2], BIT_TOV2) && GET_BIT(mcu->data[REG_TIMSK2], 0)) {
            CLR_BIT(mcu->data[REG_TIFR2], BIT_TOV2);
            PRINT_DEBUG("int timer2 ovf");
            return isr(mcu, IV_TIMER2_OVF);
        }
    }

    if (mcu->data[REG_TIMSK1]) {
        // timer1 capt (TODO)
        // timer1 compa
        if (GET_BIT(mcu->data[REG_TIFR1], BIT_OCF1A) && GET_BIT(mcu->data[REG_TIMSK1], 1)) {
            CLR_BIT(mcu->data[REG_TIFR1], BIT_OCF1A);
            PRINT_DEBUG("int timer1 compa");
            return isr(mcu, IV_TIMER1_COMPA);
        }
        // timer1 compb
        if (GET_BIT(mcu->data[REG_TIFR1], BIT_OCF1B) && GET_BIT(mcu->data[REG_TIMSK1], 2)) {
            CLR_BIT(mcu->data[REG_TIFR1], BIT_OCF1B);
            PRINT_DEBUG("int timer1 compb");
            return isr(mcu, IV_TIMER1_COMPB);
        }
        // timer1 ovf
        if (GET_BIT(mcu->data[REG_TIFR1], BIT_TOV1) && GET_BIT(mcu->data[REG_TIMSK1], 0)) {
            CLR_BIT(mcu->data[REG_TIFR1], BIT_TOV1);
            PRINT_DEBUG("int timer1 ovf");
            return isr(mcu, IV_TIMER1_OVF);
        }
    }

    if (mcu->data[REG_TIMSK0]) {
        // timer0 compa
        if (GET_BIT(mcu->data[REG_TIFR0], BIT_OCF0A) && GET_BIT(mcu->data[REG_TIMSK0], 1)) {
            CLR_BIT(mcu->data[REG_TIFR0], BIT_OCF0A);
            PRINT_DEBUG("int timer0 compa");
            return isr(mcu, IV_TIMER0_COMPA);
        }
        // timer0 compb
        if (GET_BIT(mcu->data[REG_TIFR0], BIT_OCF0B) && GET_BIT(mcu->data[REG_TIMSK0], 2)) {
            CLR_BIT(mcu->data[REG_TIFR0], BIT_OCF0B);
            PRINT_DEBUG("int timer0 compb");
            return isr(mcu, IV_TIMER0_COMPB);
        }
        // timer0 ovf
        if (GET_BIT(mcu->data[REG_TIFR0], BIT_TOV0) && GET_BIT(mcu->data[REG_TIMSK0], 0)) {
            CLR_BIT(mcu->data[REG_TIFR0], BIT_TOV0);
            PRINT_DEBUG("int timer0 ovf");
            return isr(mcu, IV_TIMER0_OVF);
        }
    }

    // spi stc

    // usart rx
    if (GET_BIT(mcu->data[REG_UCSR0B], BIT_RXCIE0) && GET_BIT(mcu->data[REG_UCSR0A], BIT_RXC0)) {
        PRINT_DEBUG("int usart rx");
        return isr(mcu, IV_USART_RX);
    }

    // usart udre
    if (GET_BIT(mcu->data[REG_UCSR0B], BIT_UDRIE0) && GET_BIT(mcu->data[REG_UCSR0A], BIT_UDRE0)) {
        PRINT_DEBUG("int usart udre");
        return isr(mcu, IV_USART_UDRE);
    }

    // usart tx
    if (GET_BIT(mcu->data[REG_UCSR0B], BIT_TXCIE0) && GET_BIT(mcu->data[REG_UCSR0A], BIT_TXC0)) {
        CLR_BIT(mcu->data[REG_UCSR0A], BIT_TXC0);
        PRINT_DEBUG("int usart tx");
        return isr(mcu, IV_USART_TX);
    }

    // adc

    // ee ready
    if (GET_BIT(mcu->data[REG_EECR], BIT_EERIE)) {
        CLR_BIT(mcu->data[REG_EECR], BIT_EERIE);
        PRINT_DEBUG("int ee ready");
        return isr(mcu, IV_EE_READY);
    }

    // analong comp
    // twi
    // spm ready

    // no interrupts were actually trigged so reset flag
    PUT_BIT(*mcu->sreg, SREG_I);

    return 0;
}

void avr_cycle(AVR_MCU *restrict mcu) {
    mcu->clk++;

    timer0_tick(mcu);
    timer1_tick(mcu);
    timer2_tick(mcu);
}
