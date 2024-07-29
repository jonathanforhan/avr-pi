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

static inline u8 xstr2byte(const char *restrict s) {
    u8 b = 0;
    b |= (isdigit(s[0]) ? s[0] - '0' : isupper(s[0]) ? s[0] - 'A' + 10 : s[0] - 'a' + 10) << 4;
    b |= (isdigit(s[1]) ? s[1] - '0' : isupper(s[1]) ? s[1] - 'A' + 10 : s[1] - 'a' + 10) << 0;
    return b;
}

// add with carry
static inline void adc(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R <- Rd + Rr + C
    const u8 R = *Rd + *Rr + GET_BIT(mcu->sreg, SREG_C);

    const u8 Rd3 = GET_BIT(*Rd, 3), Rr3 = GET_BIT(*Rr, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), Rr7 = GET_BIT(*Rr, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = Rd3 & Rr3 | Rr3 & ~R3 | ~R3 & Rd3
    SET_BIT(mcu->sreg, SREG_H, (Rd3 & Rr3) | (Rr3 & ~R3) | (~R3 & Rd3));
    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = Rd7 & Rr7 & ~R7 | ~Rd7 & ~Rr7 & R7
    SET_BIT(mcu->sreg, SREG_V, (Rd7 & Rr7 & ~R7) | (~Rd7 & ~Rr7) | R7);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, R == 0);
    // C = Rd7 & Rr7 | Rr7 & ~R7 | ~R7 & Rd7
    SET_BIT(mcu->sreg, SREG_C, (Rd7 & Rr7) | (Rr7 & ~R7) | (~R7 & Rd7));

    *Rd = R;
}

// add without carry
static inline void add(AVR_MCU *restrict mcu, u8 d, const u8 r) {
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
    SET_BIT(mcu->sreg, SREG_H, (Rd3 & Rr3) | (Rr3 & ~R3) | (~R3 & Rd3));
    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = Rd7 & Rr7 & ~R7 | ~Rd7 & ~Rr7 & R7
    SET_BIT(mcu->sreg, SREG_V, (Rd7 & Rr7 & ~R7) | (~Rd7 & ~Rr7) | R7);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, R == 0);
    // C = Rd7 & Rr7 | Rr7 & ~R7 | ~R7 & Rd7
    SET_BIT(mcu->sreg, SREG_C, (Rd7 & Rr7) | (Rr7 & ~R7) | (~R7 & Rd7));

    *Rd = R;
}

// adiw add immediate word
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
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = ~Rdh7 & R15
    SET_BIT(mcu->sreg, SREG_V, ~Rdh7 & R15);
    // N = R15
    SET_BIT(mcu->sreg, SREG_N, R15);
    // Z = ~R15 & ~R14 & ~R13 & ~R12 & ~R11 & ~R10 & ~R9 & ~R8 & ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, R == 0);
    // C = ~R15 & Rdh7
    SET_BIT(mcu->sreg, SREG_C, ~R15 & Rdh7);

    *Rd = R;
}

// and logical and
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
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, *Rd == 0);
}

// andi logical and with immediate
static inline void andi(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(K, 0, 255);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd & K
    *Rd = *Rd & K;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, *Rd == 0);
}

// asr arithmetic shift right
static inline void asr(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd >> 1
    *Rd = *Rd >> 1;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = N ^ C  **DELAYED**
    /* see NOTE */
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, *Rd == 0);
    // C = Rd0
    SET_BIT(mcu->sreg, SREG_C, GET_BIT(*Rd, 0));

    // V = N ^ C  <- NOTE N and C AFTER shift so we delay it
    SET_BIT(mcu->sreg, SREG_V, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_C));
}

// bclr bit clear in sreg
static inline void bclr(AVR_MCU *restrict mcu, u8 s) {
    ASSERT_BOUNDS(s, 0, 7);

    // SREG(s) <- 0
    CLR_BIT(mcu->sreg, s);

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

// bld bit load from the T flag in sreg to a bit in register
static inline void bld(AVR_MCU *restrict mcu, u8 d, u8 b) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    u8 *Rd = &mcu->reg[d];

    // Rd(b) <- T
    SET_BIT(*Rd, b, GET_BIT(mcu->sreg, SREG_T));

    // PC <- PC + 1
    mcu->pc++;
}

// brbc branch if bit in sreg is cleared
static inline void brbc(AVR_MCU *restrict mcu, i8 k, u8 s) {
    ASSERT_BOUNDS(k, -64, 63);
    ASSERT_BOUNDS(s, 0, 7);

    // PC <- PC + k + 1 if true
    // PC <- PC + 1 if false
    mcu->pc += GET_BIT(mcu->sreg, s) == 0 ? k + 1 : 1;
}

// brbs branch if bit in sreg is set
static inline void brbs(AVR_MCU *restrict mcu, i8 k, u8 s) {
    ASSERT_BOUNDS(k, -64, 63);
    ASSERT_BOUNDS(s, 0, 7);

    // PC <- PC + k + 1 if true
    // PC <- PC + 1 if false
    mcu->pc += GET_BIT(mcu->sreg, s) == 1 ? k + 1 : 1;
}

// break break
static inline void break_(AVR_MCU *restrict mcu) {
    mcu->pc++;
}

// bset bit set in sreg
static inline void bset(AVR_MCU *restrict mcu, u8 s) {
    ASSERT_BOUNDS(s, 0, 7);

    // SREG(s) <- 1
    PUT_BIT(mcu->sreg, s);

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

// bst bit store from bit in register to T flag in sreg
static inline void bst(AVR_MCU *restrict mcu, u8 d, u8 b) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    u8 *Rd = &mcu->reg[d];

    // T <- Rd(b)
    SET_BIT(mcu->sreg, SREG_T, GET_BIT(*Rd, b));

    // PC <- PC + 1
    mcu->pc++;

    // T = 0 if bit b in Rd is cleared. Set to 1 otherwise.
}

// call long call to a subroutine
static inline void call(AVR_MCU *restrict mcu, u16 k) {
    ASSERT_BOUNDS(k, 0, sizeof(mcu->flash) - 1);

    // PC <- k
    mcu->pc = k;

    // SP <- PC - 2
    mcu->sp -= 2;

    // STACK <- PC + 2
    *((u16 *)&mcu->data[mcu->sp]) = mcu->pc + 2;
}

// cbi clear bit in IO register
static inline void cbi(AVR_MCU *restrict mcu, u8 A, u8 b) {
    ASSERT_BOUNDS(A, 0, 31);
    ASSERT_BOUNDS(b, 0, 7);

    // IO(A, b) <- 0
    CLR_BIT(mcu->io_reg[A], b);

    // PC <- PC + 1
    mcu->pc++;
}

// com one's complement
static inline void com(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R = $FF - Rd
    *Rd = 0xFF - *Rd;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, *Rd == 0);
    // C = 1
    PUT_BIT(mcu->sreg, SREG_C);
}

// cp compare
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
    SET_BIT(mcu->sreg, SREG_H, (~Rd3 & Rr3) | (Rr3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = Rd7 & ~Rr7 & ~R7 | ~Rd7 & Rr7 & R7
    SET_BIT(mcu->sreg, SREG_V, (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7));
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, R == 0);
    // C = ~Rd7 & Rr7 | Rr7 & R7 | R7 & ~Rd7
    SET_BIT(mcu->sreg, SREG_C, (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7));
}

// cpc compare with carry
static inline void cpc(AVR_MCU *restrict mcu, u8 d, u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    const u8 *Rd = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R = Rd - Rr - C
    const u8 R = *Rd - *Rr - GET_BIT(mcu->sreg, SREG_C);

    const u8 Rd3 = GET_BIT(*Rd, 3), Rr3 = GET_BIT(*Rr, 3), R3 = GET_BIT(R, 3);
    const u8 Rd7 = GET_BIT(*Rd, 7), Rr7 = GET_BIT(*Rr, 7), R7 = GET_BIT(R, 7);

    // PC <- PC + 1
    mcu->pc++;

    // H = ~Rd3 & Rr3 | Rr3 & R3 | R3 & ~Rd3
    SET_BIT(mcu->sreg, SREG_H, (~Rd3 & Rr3) | (Rr3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = Rd7 & ~Rr7 & ~R7 | ~Rd7 & Rr7 & R7
    SET_BIT(mcu->sreg, SREG_V, (Rd7 & ~Rr7 & ~R7) | (~Rd7 & Rr7 & R7));
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0 & Z
    SET_BIT(mcu->sreg, SREG_Z, R == 0 && GET_BIT(mcu->sreg, SREG_Z));
    // C = ~Rd7 & Rr7 | Rr7 & R7 | R7 & ~Rd7
    SET_BIT(mcu->sreg, SREG_C, (~Rd7 & Rr7) | (Rr7 & R7) | (R7 & ~Rd7));
}

// cpi compare with immediate
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
    SET_BIT(mcu->sreg, SREG_H, (~Rd3 & K3) | (K3 & R3) | (R3 & ~Rd3));
    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = Rd7 & ~K7 & ~R7 | ~Rd7 & K7 & R7
    SET_BIT(mcu->sreg, SREG_V, (Rd7 & ~K7 & ~R7) | (~Rd7 & K7 & R7));
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, R7);
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, R == 0);
    // C = ~Rd7 & K7 | K7 & R7 | R7 & ~Rd7
    SET_BIT(mcu->sreg, SREG_C, (~Rd7 & K7) | (K7 & R7) | (R7 & ~Rd7));
}

// cpse compare skip if equal
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

// dec decrement
static inline void dec(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R = Rd - 1
    *Rd = *Rd - 1;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = ~R7 & R6 & R5 & R4 & R3 & R2 & R1 & R0
    SET_BIT(mcu->sreg, SREG_V, *Rd == 0x7F);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, *Rd == 0);
}

// eor exclusive or
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
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = 0
    CLR_BIT(mcu->sreg, SREG_V);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, *Rd == 0);
}

// icall indirect call to subroutine
static inline void icall(AVR_MCU *restrict mcu) {
    // PC(15:0) <- Z(15:0)
    mcu->pc = *(u16 *)&mcu->reg[REG_Z];

    // SP <- SP - 2
    mcu->sp -= 2;

    // STACK <- PC + 1
    *(u16 *)&mcu->data[mcu->sp] = mcu->pc + 1;
}

// ijmp indirect jump
static inline void ijmp(AVR_MCU *restrict mcu) {
    // PC(15:0) <- Z(15:0)
    mcu->pc = *(u16 *)&mcu->reg[REG_Z];
}

// in load an IO location to register
static inline void in(AVR_MCU *restrict mcu, u8 d, u8 A) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(A, 0, 63);

    u8 *Rd = &mcu->reg[d];

    // Rd <- IO(A)
    *Rd = mcu->io_reg[A];

    // PC <- PC + 1
    mcu->pc++;
}

// inc increment
static inline void inc(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R = Rd + 1
    *Rd = *Rd + 1;

    // PC <- PC + 1
    mcu->pc++;

    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = ~R7 & R6 & R5 & R4 & R3 & R2 & R1 & R0
    SET_BIT(mcu->sreg, SREG_V, *Rd == 0x80);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, *Rd == 0);
}

// jmp jump
static inline void jmp(AVR_MCU *restrict mcu, u16 k) {
    // PC <- k
    mcu->pc = k;
}

// lac load and clear
static inline void lac(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd  = &mcu->reg[d];
    u16 *Z  = (u16 *)&mcu->reg[REG_Z];
    u16 tmp = *Z;

    // (Z) <- ($FF - Rd) & (Z), Rd <- (Z)
    *Z  = (0xFF - *Rd) & *Z;
    *Rd = tmp;

    // PC <- PC + 1
    mcu->pc++;
}

// las load and clear
static inline void las(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd  = &mcu->reg[d];
    u16 *Z  = (u16 *)&mcu->reg[REG_Z];
    u16 tmp = *Z;

    // (Z) <- Rd | (Z), Rd <- (Z)
    *Z  = *Rd | *Z;
    *Rd = tmp;

    // PC <- PC + 1
    mcu->pc++;
}

// lat load and toggle
static inline void lat(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd  = &mcu->reg[d];
    u16 *Z  = (u16 *)&mcu->reg[REG_Z];
    u16 tmp = *Z;

    // (Z) <- Rd ^ (Z), Rd <- (Z)
    *Z  = *Rd ^ *Z;
    *Rd = tmp;

    // PC <- PC + 1
    mcu->pc++;
}

void avr_mcu_init(AVR_MCU *restrict mcu) {
    memset(mcu, 0, sizeof(*mcu));
    mcu->sp = sizeof(mcu->data);
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
        case DATA_RECORD: {
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
            LOG_ERROR("TODO");
            return AVR_ERROR;
        }
    }

    return AVR_ERROR;
}

void avr_cycle(AVR_MCU *const restrict mcu) {
    const u16 op = mcu->flash[mcu->pc];

    /***************************************************************************
     * 4 bit op
     **************************************************************************/
    switch (op & OP_MASK_4) {
    case OP_ANDI: {
        const u8 d = (op & 0x00F0) >> 4;
        const u8 K = ((op & 0x0F00) >> 4) | (op & 0x000F);
        andi(mcu, d, K);
        return;
    }
    case OP_CPI: {
        const u8 d = (op & 0x00F0) >> 4;
        const u8 K = ((op & 0x0F00) >> 4) | (op & 0x000F);
        cpi(mcu, d, K);
        return;
    }
    }

    /***************************************************************************
     * 5 bit op
     **************************************************************************/
    switch (op & OP_MASK_5) {
    case OP_IN: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 A = ((op & 0x600) >> 5) | (op & 0x000F);
        in(mcu, d, A);
        return;
    }
    }

    /***************************************************************************
     * 6 bit op
     **************************************************************************/
    switch (op & OP_MASK_6) {
    case OP_ADC: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 r = GET_REG_DIRECT_SRC(op);
        adc(mcu, d, r);
        return;
    }
    case OP_BRBC: {
        const i8 k = (i8)(64 - ((op & 0x03F8) >> 3));
        const u8 s = op & 0x0003;
        brbc(mcu, k, s);
        return;
    }
    case OP_BRBS: {
        const i8 k = (i8)(64 - ((op & 0x03F8) >> 3));
        const u8 s = op & 0x0003;
        brbs(mcu, k, s);
        return;
    }
    case OP_ADD: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 r = GET_REG_DIRECT_SRC(op);
        add(mcu, d, r);
        return;
    }
    case OP_AND: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 r = GET_REG_DIRECT_SRC(op);
        and(mcu, d, r);
        return;
    }
    case OP_CP: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 r = GET_REG_DIRECT_SRC(op);
        cp(mcu, d, r);
        return;
    }
    case OP_CPC: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 r = GET_REG_DIRECT_SRC(op);
        cpc(mcu, d, r);
        return;
    }
    case OP_CPSE: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 r = GET_REG_DIRECT_SRC(op);
        cpse(mcu, d, r);
        return;
    }
    case OP_EOR: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 r = GET_REG_DIRECT_SRC(op);
        eor(mcu, d, r);
        return;
    }
    }

    /***************************************************************************
     * 7 bit op
     **************************************************************************/
    switch (op & OP_MASK_7_1) {
    case OP_BLD: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 b = op & 0x0003;
        bld(mcu, d, b);
        return;
    }
    case OP_BST: {
        const u8 d = GET_REG_DIRECT_DST(op);
        const u8 b = op & 0x0003;
        bst(mcu, d, b);
        return;
    }
    }
    switch (op & OP_MASK_7_3) {
    case OP_CALL: {
        const u16 k = mcu->flash[mcu->pc + 1];
        call(mcu, k);
        return;
    }
    case OP_JMP: {
        const u16 k = mcu->flash[mcu->pc + 1];
        jmp(mcu, k);
        return;
    }
    }
    switch (op & OP_MASK_7_4) {
    case OP_ASR: {
        const u8 d = GET_REG_DIRECT_DST(op);
        asr(mcu, d);
        return;
    }
    case OP_DEC: {
        const u8 d = GET_REG_DIRECT_DST(op);
        dec(mcu, d);
        return;
    }
    case OP_INC: {
        const u8 d = GET_REG_DIRECT_DST(op);
        inc(mcu, d);
        return;
    }
    case OP_LAC: {
        const u8 d = GET_REG_DIRECT_DST(op);
        lac(mcu, d);
        return;
    }
    case OP_LAS: {
        const u8 d = GET_REG_DIRECT_DST(op);
        las(mcu, d);
        return;
    }
    case OP_LAT: {
        const u8 d = GET_REG_DIRECT_DST(op);
        lat(mcu, d);
        return;
    }
    }

    /***************************************************************************
     * 8 bit op
     **************************************************************************/
    switch (op & OP_MASK_8) {
    case OP_ADIW: {
        const u8 d = (op & 0x0030) >> 4;
        const u8 K = ((op & 0x00C0) >> 2) | (op & 0x000F);
        adiw(mcu, d, K);
        return;
    }
    case OP_CBI: {
        const u8 A = (op & 0x00F8) >> 3;
        const u8 b = op & 0x0003;
        cbi(mcu, A, b);
        return;
    }
    }

    /***************************************************************************
     * 9 bit op
     **************************************************************************/
    switch (op & OP_MASK_9_4) {
    case OP_BCLR: {
        const u8 s = (op & ~OP_MASK_9_4) >> 4;
        bclr(mcu, s);
        return;
    }
    case OP_BSET: {
        const u8 s = (op & ~OP_MASK_9_4) >> 4;
        bset(mcu, s);
        return;
    }
    case OP_COM: {
        const u8 d = GET_REG_DIRECT_DST(op);
        com(mcu, d);
        return;
    }
    }

    /***************************************************************************
     * 16 bit op
     **************************************************************************/
    switch (op) {
    case OP_BREAK:
        break_(mcu);
        return;
    case OP_ICALL:
        icall(mcu);
        return;
    case OP_IJMP:
        ijmp(mcu);
        return;
    }

    LOG_ERROR("unknown instruction %#x", op);
    exit(EXIT_FAILURE);
}
