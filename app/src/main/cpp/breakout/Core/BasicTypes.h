/*
MIT License

Copyright (c) 2023 Victor Falcon Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef _BASIC_TYPES_H_
#define _BASIC_TYPES_H_

#include <stdint.h>

// Signed byte type
typedef signed char ibyte;
// Unsigned byte type
typedef unsigned char ubyte;

// Signed types

// Singed integer 8 bit type
typedef int8_t i8;
// Singed integer 16 bit type
typedef int16_t i16;
// Singed integer 32 bit type
typedef int32_t i32;
// Singed integer 64 bit type
typedef int64_t i64;

constexpr i8 I8_MAX = INT8_MAX;        /**< Maximum value for a signed 8 bit integer */
constexpr i16 I16_MAX = INT16_MAX;    /**< Maximum value for a signed 16 bit integer */
constexpr i32 I32_MAX = INT32_MAX;    /**< Maximum value for a signed 32 bit integer */
constexpr i64 I64_MAX = INT64_MAX;    /**< Maximum value for a signed 64 bit integer */

// Unsigned
// Unsigned integer 8 bit type
typedef uint8_t u8;
// Unsigned integer 16 bit type
typedef uint16_t u16;
// Unsigned integer 32 bit type
typedef uint32_t u32;
// Unsigned integer 64 bit type
typedef uint64_t u64;

constexpr u8 U8_MAX = UINT8_MAX;        /**< Maximum value for an unsigned 8 bit integer */
constexpr u16 U16_MAX = UINT16_MAX;    /**< Maximum value for an unsigned 16 bit integer */
constexpr u32 U32_MAX = UINT32_MAX;    /**< Maximum value for an unsigned 32 bit integer */
constexpr u64 U64_MAX = UINT64_MAX;    /**< Maximum value for an unsigned 64 bit integer */

// Floating pointer type

// Singed floating pointer 32 bit type
typedef float f32;

// Singed floating pointer 64 bit type
typedef double f64;
#endif