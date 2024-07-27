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
#include "avr_defs.h"
#include "defs.h"

static inline u8 xstr2byte(const char *restrict s) {
    u8 b = 0;
    b |= (isdigit(s[0]) ? s[0] - '0' : isupper(s[0]) ? s[0] - 'A' + 10 : s[0] - 'a' + 10) << 4;
    b |= (isdigit(s[1]) ? s[1] - '0' : isupper(s[1]) ? s[1] - 'A' + 10 : s[1] - 'a' + 10) << 0;
    return b;
}

int avr_program(AVR_MCU *restrict mcu, const char *restrict hex) {
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

            checksum = ~checksum + 1; // two's complement

            if (checksum != xstr2byte(hex)) {
                LOG_ERROR("checksum failure: real %#x, expected %#x, len %#x, addr %#x, type %#x",
                          checksum,
                          xstr2byte(hex),
                          len,
                          addr,
                          type);
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

    switch (op & OP_MASK_6) {
    case OP_ADC: {
        u8 *Rd       = &mcu->reg[GET_REG_DIRECT_DST(op)];
        const u8 *Rr = &mcu->reg[GET_REG_DIRECT_SRC(op)];

        const u8 R = *Rd + *Rr + GET_BIT(mcu->sreg, SREG_C);

        // PC <- PC + 1
        mcu->pc++;

        // H = Rd3 & Rr3 | Rr3 & ~R3 | ~R3 & Rd3

        // S = N ^ V

        LOG_DEBUG("adc %#x,%#x", *Rd, *Rr);
    }
        return;
    default:
        LOG_DEBUG("unknown instruction %#x", op);
        return;
    }

    switch (op & OP_MASK_7) {
    case OP_TYPE_JUMP:
        switch (op & OP_MASK_JUMP) {
        case OP_JMP:
            mcu->pc = GET_PROG_DIRECT_ADDR(op, mcu->flash[mcu->pc + 1]);
            LOG_DEBUG("jmp %#x", mcu->pc * 2);
            return;
        default:
            LOG_DEBUG("unknown instruction %#x", op);
            return;
        }
    default:
        LOG_DEBUG("unknown instruction %#x", op);
        return;
    }
}
