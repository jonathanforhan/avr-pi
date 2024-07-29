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
#include <stdint.h>
#include <stdio.h>

#include <avr.c> // NOLINT(bugprone-suspicious-include)

static AVR_Result test_arithmetic_and_logic_instructions(void) {
    AVR_MCU mcu;
    avr_mcu_init(&mcu);

    // add
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[0] = i, mcu.reg[1] = j;

            u8 expected = mcu.reg[0] + mcu.reg[1];

            add(&mcu, 0, 1);

            u8 real = mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed add: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // adc
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            PUT_BIT(mcu.sreg, SREG_C);
            mcu.reg[0] = i, mcu.reg[1] = j;

            u8 expected = mcu.reg[0] + mcu.reg[1] + 1;

            adc(&mcu, 0, 1);

            u8 real = mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed adc: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // adiw
    for (int i = 0; i < INT16_MAX / 4; i += 8) {
        for (int j = 0; j < 64; j += 4) {
            *(u16 *)&mcu.reg[30] = i;

            u16 expected = i + j;

            adiw(&mcu, 3, j);

            u16 real = *(u16 *)&mcu.reg[30];

            if (real != expected) {
                LOG_ERROR("test failed adiw: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // sub
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[0] = i, mcu.reg[1] = j;

            u8 expected = mcu.reg[0] - mcu.reg[1];

            sub(&mcu, 0, 1);

            u8 real = mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed sub: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // subi
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[16] = i;

            u8 expected = mcu.reg[16] - j;

            subi(&mcu, 0, j);

            u8 real = mcu.reg[16];

            if (real != expected) {
                LOG_ERROR("test failed subi: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // sbc
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            PUT_BIT(mcu.sreg, SREG_C);
            mcu.reg[0] = i, mcu.reg[1] = j;

            u8 expected = mcu.reg[0] - mcu.reg[1] - 1;

            sbc(&mcu, 0, 1);

            u8 real = mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed sbc: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // sbci
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            PUT_BIT(mcu.sreg, SREG_C);
            mcu.reg[16] = i;

            u8 expected = mcu.reg[16] - j - 1;

            sbci(&mcu, 0, j);

            u8 real = mcu.reg[16];

            if (real != expected) {
                LOG_ERROR("test failed sbci: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // sbiw
    for (u16 i = 0; i < INT16_MAX / 4; i += 8) {
        for (u16 j = 0; j < 64; j += 4) {
            *(u16 *)&mcu.reg[30] = i;

            u16 expected = i - j;

            sbiw(&mcu, 3, j);

            u16 real = *(u16 *)&mcu.reg[30];

            if (real != expected) {
                LOG_ERROR("test failed sbiw: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // and
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[0] = i, mcu.reg[1] = j;

            u8 expected = mcu.reg[0] & mcu.reg[1];

            and(&mcu, 0, 1);

            u8 real = mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed and: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // andi
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[16] = i;

            u8 expected = mcu.reg[16] & j;

            andi(&mcu, 0, j);

            u8 real = mcu.reg[16];

            if (real != expected) {
                LOG_ERROR("test failed andi: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // or
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[0] = i, mcu.reg[1] = j;

            u8 expected = mcu.reg[0] | mcu.reg[1];

            or (&mcu, 0, 1);

            u8 real = mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed or: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // ori
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[16] = i;

            u8 expected = mcu.reg[16] | j;

            ori(&mcu, 0, j);

            u8 real = mcu.reg[16];

            if (real != expected) {
                LOG_ERROR("test failed ori: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // eor
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[0] = i, mcu.reg[1] = j;

            u8 expected = mcu.reg[0] ^ mcu.reg[1];

            eor(&mcu, 0, 1);

            u8 real = mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed eor: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // com
    for (u16 i = 0; i < 256; i++) {
        mcu.reg[0] = i;

        u8 expected = 0xFF - mcu.reg[0];

        com(&mcu, 0);

        u8 real = mcu.reg[0];

        if (real != expected) {
            LOG_ERROR("test failed com: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // neg
    for (u16 i = 0; i < 256; i++) {
        mcu.reg[0] = i;

        u8 expected = 0x00 - mcu.reg[0];

        neg(&mcu, 0);

        u8 real = mcu.reg[0];

        if (real != expected) {
            LOG_ERROR("test failed neg: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // sbr
    // ori equivalent

    // cbr
    // andi equivalent

    // inc
    for (u16 i = 0; i < 255; i++) {
        if (i == 0) {
            mcu.reg[0] = 0;
        }

        u8 expected = mcu.reg[0] + 1;

        inc(&mcu, 0);

        u8 real = mcu.reg[0];

        if (real != expected) {
            LOG_ERROR("test failed inc: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // dec
    for (u16 i = 0; i < 255; i++) {
        if (i == 0) {
            mcu.reg[0] = 255;
        }

        u8 expected = mcu.reg[0] - 1;

        dec(&mcu, 0);

        u8 real = mcu.reg[0];

        if (real != expected) {
            LOG_ERROR("test failed dec: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // tst
    // and equivalent

    // clr
    // eor equivalent

    // ser
    {
        mcu.reg[16] = 0;

        u8 expected = 0xFF;

        ser(&mcu, 0);

        u8 real = mcu.reg[16];

        if (real != expected) {
            LOG_ERROR("test failed ser: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // mul
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[0] = i, mcu.reg[1] = j;

            u16 expected = i * j;

            mul(&mcu, 0, 1);

            u16 real = *(u16 *)&mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed mul: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // muls
    for (i16 i = -128; i < 127; i++) {
        for (i16 j = -128; j < 127; j++) {
            *(i8 *)&mcu.reg[16] = (i8)i, *(i8 *)&mcu.reg[17] = (i8)j;

            i16 expected = (i16)(i * j);

            muls(&mcu, 0, 1);

            i16 real = *(i16 *)&mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed muls: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // mulsu
    for (i16 i = -128; i < 127; i++) {
        for (u16 j = 0; j < 255; j++) {
            *(i8 *)&mcu.reg[16] = (i8)i, mcu.reg[17] = j;

            i16 expected = (i16)(i * j);

            mulsu(&mcu, 0, 1);

            i16 real = *(i16 *)&mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed mulsu: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // fmul
    for (u16 i = 0; i < 256; i++) {
        for (u16 j = 0; j < 256; j++) {
            mcu.reg[16] = i, mcu.reg[17] = j;

            u16 expected = (i * j) << 1;

            fmul(&mcu, 0, 1);

            u16 real = *(u16 *)&mcu.reg[0];

            if (real != expected) {
                LOG_DEBUG("%u,%u", (u8)i, (u8)j);
                LOG_ERROR("test failed fmul: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // fmuls
    for (i16 i = -128; i < 127; i++) {
        for (i16 j = -128; j < 127; j++) {
            *(i8 *)&mcu.reg[16] = (i8)i, *(i8 *)&mcu.reg[17] = (i8)j;

            i16 expected = (i16)((i * j) << 1);

            fmuls(&mcu, 0, 1);

            i16 real = *(i16 *)&mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed fmuls: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    // fmulsu
    for (i16 i = -128; i < 127; i++) {
        for (u16 j = 0; j < 255; j++) {
            *(i8 *)&mcu.reg[16] = (i8)i, mcu.reg[17] = j;

            i16 expected = (i16)((i * j) << 1);

            fmulsu(&mcu, 0, 1);

            i16 real = *(i16 *)&mcu.reg[0];

            if (real != expected) {
                LOG_ERROR("test failed fmulsu: real %#x, expected %#x", real, expected);
                return AVR_ERROR;
            }
        }
    }

    return AVR_OK;
}

static AVR_Result avr_run_tests(void) {
    AVR_MCU mcu;
    avr_mcu_init(&mcu);

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

    // cbi
    for (int i = 0; i < 8; i++) {
        mcu.io_reg[0] = 0xFF;

        u8 expected = mcu.io_reg[0] & ~(1 << i);

        cbi(&mcu, 0, i);

        u8 real = mcu.io_reg[0];

        if (real != expected) {
            LOG_ERROR("test failed cbi: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // cp
    for (int i = 0; i < 2; i++) {
        mcu.reg[0] = 0;
        mcu.reg[1] = i;

        u8 expected = mcu.reg[0] == mcu.reg[1];

        cp(&mcu, 0, 1);

        u8 real = GET_BIT(mcu.sreg, SREG_Z);

        if (real != expected) {
            LOG_ERROR("test failed cp: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // cpc
    for (int i = 0; i < 2; i++) {
        PUT_BIT(mcu.sreg, SREG_C);
        PUT_BIT(mcu.sreg, SREG_Z);

        mcu.reg[0] = 1;
        mcu.reg[1] = i;

        u8 expected = mcu.reg[0] == mcu.reg[1] + 1;

        cpc(&mcu, 0, 1);

        u8 real = GET_BIT(mcu.sreg, SREG_Z);

        if (real != expected) {
            LOG_ERROR("test failed cpc: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // cpi
    for (int i = 0; i < 255; i++) {
        mcu.reg[16] = 42;

        u8 expected = mcu.reg[16] == i;

        cpi(&mcu, 0, i);

        u8 real = GET_BIT(mcu.sreg, SREG_Z);

        if (real != expected) {
            LOG_ERROR("test failed cpi: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // cpse
    // TODO hard to test without avr code

    // icall
    // TODO hard to test without avr code

    // ijmp
    // TODO hard to test without avr code

    // in
    {
        mcu.io_reg[63] = 42;

        u8 expected = mcu.io_reg[63];

        in(&mcu, 0, 63);

        u8 real = mcu.reg[0];

        if (real != expected) {
            LOG_ERROR("test failed in: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // jmp
    // no need to test

    // lac
    // TODO

    // las
    // TODO

    // lat
    // TODO

    // ld
    for (int i = 0; i < 256; i++) {
        *(u16 *)&mcu.reg[REG_X] = i + 256;
        mcu.data[i + 256]       = i;

        u8 expected = i;

        ld_x(&mcu, 0);

        u8 real = mcu.reg[0];
        if (real != expected) {
            LOG_ERROR("test failed ld: real %#x, expected %#x", real, expected);
            return AVR_ERROR;
        }
    }

    // ldi
    // no need to test

    // lds
    // no need to test

    return AVR_OK;
}

int main(void) {
    if (test_arithmetic_and_logic_instructions() != AVR_OK) {
        printf("tests failed\n");
        return -1;
    } else {
        printf("tests ran successfully\n");
        return 0;
    }
}
