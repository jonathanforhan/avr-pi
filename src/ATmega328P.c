#include "ATmega328P.h"

#include <assert.h>
#include <ctype.h>
#include "AVR.h"
#include "common.h"

static inline u8 xstr2byte(const char s[2]) {
    u8 b = 0;
    b |= (isdigit(s[0]) ? s[0] - '0' : isupper(s[0]) ? s[0] - 'A' + 10 : s[0] - 'a' + 10) << 4;
    b |= (isdigit(s[1]) ? s[1] - '0' : isupper(s[1]) ? s[1] - 'A' + 10 : s[1] - 'a' + 10) << 0;
    return b;
}

bool ATmega238P_WriteProgram(ATmega238P_Memory* memory, const char* hex) {
    assert(memory && hex);

    while (*hex) {
        if (*hex != ':') {
            hex++;
            continue;
        }

        hex++;

        u8 len = xstr2byte(hex);
        hex += 2;

        u16 addr = (xstr2byte(hex) << 8) | xstr2byte(hex + 2);
        hex += 4;

        u8 type = xstr2byte(hex);
        hex += 2;

        switch (type) {
        case DATA_RECORD: {
            u8 checksum = len + (addr >> 8) + (addr & 0x00FF) + type;

            for (int i = 0; i < len; i++) {
                u8 b = xstr2byte(hex);
                hex += 2;

                ((u8*)memory->flash)[addr + i] = b;
                checksum += b;
            }

            checksum = ((~checksum) + 1) & 0xFF; // two's complement

            if (checksum != xstr2byte(hex)) {
                LOG_ERROR("checksum failure");
                return false;
            }
            hex += 2;
        } break;
        case EOF_RECORD:
            return true;
        default:
            LOG_ERROR("TODO");
            return false;
        }
    }

    return false;
}
