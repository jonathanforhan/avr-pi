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

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "avr_defs.h"
#include "defs.h"

#if defined __GNUC__ && !defined NDEBUG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
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
// TODO

// and logical and
static inline void and (AVR_MCU *restrict mcu, u8 d, const u8 r) {
    ASSERT_BOUNDS(d, 0, 31);
    ASSERT_BOUNDS(r, 0, 31);

    u8 *Rd       = &mcu->reg[d];
    const u8 *Rr = &mcu->reg[r];

    // R <- Rd & Rr
    const u8 R = *Rd & *Rr;

    // PC <- PC + 1
    mcu->pc++;
    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = 0
    SET_BIT(mcu->sreg, SREG_V, 0);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, R == 0);

    *Rd = R;
}

// andi logical and with immediate
static inline void andi(AVR_MCU *restrict mcu, u8 d, u8 K) {
    ASSERT_BOUNDS(d, 16, 31);
    ASSERT_BOUNDS(K, 0, 255);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd & K
    const u8 R = *Rd & K;

    // PC <- PC + 1
    mcu->pc++;
    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = 0
    SET_BIT(mcu->sreg, SREG_V, 0);
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, R == 0);

    *Rd = R;
}

// asr arithmetic shift right
static inline void asr(AVR_MCU *restrict mcu, u8 d) {
    ASSERT_BOUNDS(d, 0, 31);

    u8 *Rd = &mcu->reg[d];

    // R <- Rd >> 1
    const u8 R = *Rd >> 1;

    // PC <- PC + 1
    mcu->pc++;
    // S = N ^ V
    SET_BIT(mcu->sreg, SREG_S, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_V));
    // V = N ^ C  **DELAYED**
    /* see NOTE */
    // N = R7
    SET_BIT(mcu->sreg, SREG_N, GET_BIT(*Rd, 7));
    // Z = ~R7 & ~R6 & ~R5 & ~R4 & ~R3 & ~R2 & ~R1 & ~R0
    SET_BIT(mcu->sreg, SREG_Z, R == 0);
    // C = Rd0
    SET_BIT(mcu->sreg, SREG_C, GET_BIT(*Rd, 0));

    // V = N ^ C  <- NOTE N and C AFTER shift so we delay it
    SET_BIT(mcu->sreg, SREG_V, GET_BIT(mcu->sreg, SREG_N) ^ GET_BIT(mcu->sreg, SREG_C));

    *Rd = R;
}

// bclr bit clear in sreg
static inline void bclr(AVR_MCU *restrict mcu, u8 s) {
    ASSERT_BOUNDS(s, 0, 7);

    // SREG(s) <- 0
    SET_BIT(mcu->sreg, s, 0);

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
    SET_BIT(mcu->sreg, s, 1);

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

void avr_mcu_init(AVR_MCU *restrict mcu) {
    memcpy(mcu, &(AVR_MCU){0}, sizeof(*mcu));
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
        u8 d = (op & 0x00F0) >> 4;
        u8 K = ((op & 0x0F00) >> 4) | (op & 0x000F);
        return andi(mcu, d, K);
    }
    }

    /***************************************************************************
     * 6 bit op
     **************************************************************************/
    switch (op & OP_MASK_6) {
    case OP_ADC: {
        u8 d = GET_REG_DIRECT_DST(op);
        u8 r = GET_REG_DIRECT_SRC(op);
        return adc(mcu, d, r);
    }
    case OP_BRBC: {
        i8 k = (i8)(64 - ((op & 0b000001111111000) >> 3));
        u8 s = op & 0b111;
        return brbc(mcu, k, s);
    }
    case OP_BRBS: {
        i8 k = (i8)(64 - ((op & 0b000001111111000) >> 3));
        u8 s = op & 0b111;
        return brbs(mcu, k, s);
    }
    case OP_ADD: {
        u8 d = GET_REG_DIRECT_DST(op);
        u8 r = GET_REG_DIRECT_SRC(op);
        return add(mcu, d, r);
    }
    case OP_AND: {
        u8 d = GET_REG_DIRECT_DST(op);
        u8 r = GET_REG_DIRECT_SRC(op);
        return and(mcu, d, r);
    }
    }

    /***************************************************************************
     * 7 bit op
     **************************************************************************/
    switch (op & OP_MASK_7_1) {
    case OP_BLD: {
        u8 d = GET_REG_DIRECT_DST(op);
        u8 b = op & 0b111;
        return bld(mcu, d, b);
    }
    case OP_BST: {
        u8 d = GET_REG_DIRECT_DST(op);
        u8 b = op & 0b111;
        return bst(mcu, d, b);
    }
    }
    switch (op & OP_MASK_7_3) {
    case OP_CALL: {
        u16 k = (u16)GET_PROG_DIRECT_ADDR(op, mcu->flash[mcu->pc + 1]);
        return call(mcu, k);
    }
#if 0
    case OP_JMP: {
        // TODO
        mcu->pc = GET_PROG_DIRECT_ADDR(op, mcu->flash[mcu->pc + 1]);
        return;
    }
#endif
    }
    switch (op & OP_MASK_7_4) {
    case OP_ASR: {
        u8 d = GET_REG_DIRECT_DST(op);
        return asr(mcu, d);
    }
    }

    /***************************************************************************
     * 8 bit op
     **************************************************************************/
    switch (op & OP_MASK_8) {
    case OP_ADIW: {
        LOG_DEBUG("TODO - ADIW");
        return;
    }
    }

    /***************************************************************************
     * 9 bit op
     **************************************************************************/
    switch (op & OP_MASK_9_4) {
    case OP_BCLR: {
        u8 s = (op & ~OP_MASK_9_4) >> 4;
        return bclr(mcu, s);
    }
    case OP_BSET: {
        u8 s = (op & ~OP_MASK_9_4) >> 4;
        return bset(mcu, s);
    }
    }

    /***************************************************************************
     * 16 bit op
     **************************************************************************/
    switch (op) {
    case OP_BREAK:
        return break_(mcu);
    }

    LOG_DEBUG("unknown instruction %#x", op);
    exit(EXIT_FAILURE);
}

#ifdef AVR_TEST
AVR_Result avr_run_tests(void) {
    AVR_MCU mcu;
    avr_mcu_init(&mcu);

    // adc
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            SET_BIT(mcu.sreg, SREG_C, 1);
            mcu.reg[0] = i, mcu.reg[1] = j;

            u16 expected = (u16)mcu.reg[0] + (u16)mcu.reg[1] + 1;

            adc(&mcu, 0, 1);

            u16 real = (u16)mcu.reg[0] + (GET_BIT(mcu.sreg, SREG_C) ? 0x100 : 0);

            if (real != expected) {
                LOG_ERROR("test failed adc: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // add
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            mcu.reg[0] = i, mcu.reg[1] = j;

            u32 expected = (u32)mcu.reg[0] + (u32)mcu.reg[1];

            add(&mcu, 0, 1);

            u32 real = (u32)mcu.reg[0] + (GET_BIT(mcu.sreg, SREG_C) ? 0x100 : 0);

            if (real != expected) {
                LOG_ERROR("test failed add: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // adiw
    // TODO

    // and
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            mcu.reg[0] = i, mcu.reg[1] = j;

            u8 expected = mcu.reg[0] & mcu.reg[1];

            and(&mcu, 0, 1);

            u8 real = mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed add: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // andi
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            u8 K        = j;
            mcu.reg[16] = i;

            u8 expected = mcu.reg[16] & K;

            andi(&mcu, 16, K);

            u8 real = mcu.reg[16];

            if (real != expected) {
                LOG_ERROR("test failed addi: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // asr
    for (int i = 0; i < 256; i++) {
        mcu.reg[0] = i;

        u8 expected = mcu.reg[0] >> 1;

        asr(&mcu, 0);

        u8 real = mcu.reg[0];

        if (real != expected) {
            LOG_ERROR("test failed asr: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // bclr
    for (int i = 0; i < 8; i++) {
        u8 expected = mcu.sreg & ~(1 << i);

        bclr(&mcu, i);

        u8 real = mcu.sreg;

        if (real != expected) {
            LOG_ERROR("test failed bclr: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // bld
    for (int i = 0; i < 8; i++) {
        mcu.reg[0] = 0;
        mcu.sreg   = 0;

        const u8 T = i % 2;
        SET_BIT(mcu.sreg, SREG_T, T);

        u8 expected = mcu.reg[0];
        SET_BIT(expected, i, T);

        bld(&mcu, 0, i);

        u8 real = mcu.reg[0];

        if (real != expected) {
            LOG_ERROR("test failed bld: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // brbc
    for (int i = 0; i < 8; i++) {
        mcu.sreg = 0b11110000;
        mcu.pc   = 0;
        i8 k     = 42;

        u8 expected = GET_BIT(mcu.sreg, i) == 0 ? mcu.pc + k + 1 : mcu.pc + 1;

        brbc(&mcu, k, i);

        u8 real = mcu.pc;

        if (real != expected) {
            LOG_ERROR("test failed brbc: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // brbs
    for (int i = 0; i < 8; i++) {
        mcu.sreg = 0b11110000;
        mcu.pc   = 0;
        i8 k     = 42;

        u8 expected = GET_BIT(mcu.sreg, i) == 1 ? mcu.pc + k + 1 : mcu.pc + 1;

        brbs(&mcu, k, i);

        u8 real = mcu.pc;

        if (real != expected) {
            LOG_ERROR("test failed brbs: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // bset
    for (int i = 0; i < 8; i++) {
        u8 expected = mcu.sreg | (1 << i);

        bset(&mcu, i);

        u8 real = mcu.sreg;

        if (real != expected) {
            LOG_ERROR("test failed bset: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // bst
    for (int i = 0; i < 8; i++) {
        mcu.reg[0] = (1 << i);
        mcu.sreg   = 0;

        u8 expected = 1 << SREG_T;

        bst(&mcu, 0, i);

        u8 real = mcu.sreg;

        if (real != expected) {
            LOG_ERROR("test failed bst: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // call
    for (u32 i = 0; i < (sizeof(mcu.data) / 2); i++) {
        u16 expected     = i;
        u16 expected_sp  = mcu.sp - 2;
        u16 expected_stk = i + 2;

        call(&mcu, i);

        u16 real     = mcu.pc;
        u16 real_sp  = mcu.sp;
        u16 real_stk = *(u16 *)&mcu.data[mcu.sp];

        if (real != expected) {
            LOG_ERROR("test failed call: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        } else if (real_sp != expected_sp) {
            LOG_ERROR("test failed call: real_sp %#x, expected_sp %#x", real_sp, expected_sp);
            return AVR_ERROR;
        } else if (real_stk != expected_stk) {
            LOG_ERROR("test failed call: real_stk %#x, expected_stk %#x", real_stk, expected_stk);
            return AVR_ERROR;
        }
    }

    return AVR_OK;
}
#endif
