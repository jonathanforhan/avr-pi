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

#pragma once

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define LOG_ERROR(MSG, ...) (void)fprintf(stderr, "%s:%d ERROR " MSG "\n", __func__, __LINE__, ##__VA_ARGS__)

#ifdef NDEBUG
#define LOG_DEBUG(MSG, ...) (void)0
#else
#define LOG_DEBUG(MSG, ...) (void)fprintf(stderr, "%s:%d DEBUG " MSG "\n", __func__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef NDEBUG
#define PRINT_DEBUG(...) (void)0
#else
#define PRINT_DEBUG(...) (void)printf(__VA_ARGS__)
// #define PRINT_DEBUG(...) (void)0
#endif

// get Nth bit from X returns 0 or 1
#define GET_BIT(X, N) (((X) >> (N)) & 1)

// set Nth bit in X to boolean value V
#define SET_BIT(X, N, V) (X = (X & ~(1 << (N))) | ((!!(V)) << (N)))

// put Nth bit in X to high
#define PUT_BIT(X, N) ((X) |= (1 << (N)))

// clear Nth bit from X
#define CLR_BIT(X, N) (X &= ~(1 << (N)))

// returns two's complement
#define TWO_COMP(X) (~(X) + 1)

// convert a 12bit number to i16
#define I12_TO_I16(X) ((X) | GET_BIT((X), 11) * 0xF000)

// convert a 7bit number to i16
#define I7_TO_I16(X) ((X) | GET_BIT((X), 6) * 0xFF80)

// assert X is equal to or between LO and HI
#define ASSERT_BOUNDS(X, LO, HI) assert((X) >= (LO) && (X) <= (HI))

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
