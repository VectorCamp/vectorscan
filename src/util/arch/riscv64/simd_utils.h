/*
 * Copyright (c) 2015-2020, Intel Corporation
 * Copyright (c) 2020-2026, VectorCamp PC
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** \file
 * \brief SIMD types and primitive operations for RISC-V 64-bit.
 *
 * m128 union members: b=bytes(16), h=halfwords(8), w=words(4), d=doublewords(2).
 * Uses scalar C for correctness; can be accelerated with RVV intrinsics later.
 */

#ifndef ARCH_RISCV64_SIMD_UTILS_H
#define ARCH_RISCV64_SIMD_UTILS_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "ue2common.h"
#include "util/simd_types.h"
#include "util/unaligned.h"
#include "util/intrinsics.h"

/*** 128-bit primitives ***/

static really_inline m128 ones128(void) {
    m128 rv;
    memset(&rv, 0xFF, sizeof(rv));
    return rv;
}

static really_inline m128 zeroes128(void) {
    m128 rv;
    memset(&rv, 0, sizeof(rv));
    return rv;
}

static really_inline m128 not128(m128 a) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = ~a.b[i];
    return rv;
}

static really_inline int diff128(m128 a, m128 b) {
    for (int i = 0; i < 16; i++)
        if (a.b[i] != b.b[i])
            return 1;
    return 0;
}

static really_inline int isnonzero128(m128 a) {
    return diff128(a, zeroes128());
}

static really_inline u32 diffrich128(m128 a, m128 b) {
    u32 mask = 0;
    for (int i = 0; i < 4; i++)
        if (a.w[i] != b.w[i])
            mask |= (1U << i);
    return mask;
}

static really_inline u32 diffrich64_128(m128 a, m128 b) {
    u32 mask = 0;
    for (int i = 0; i < 2; i++)
        if (a.d[i] != b.d[i])
            mask |= (1U << (2 * i));
    return mask;
}

static really_really_inline
m128 add_2x64(m128 a, m128 b) {
    m128 rv;
    rv.d[0] = a.d[0] + b.d[0];
    rv.d[1] = a.d[1] + b.d[1];
    return rv;
}

static really_really_inline
m128 sub_2x64(m128 a, m128 b) {
    m128 rv;
    rv.d[0] = a.d[0] - b.d[0];
    rv.d[1] = a.d[1] - b.d[1];
    return rv;
}

static really_inline
m128 lshift_m128(m128 a, unsigned b) {
    m128 rv = zeroes128();
    if (b < 32) {
        for (int i = 0; i < 4; i++)
            rv.w[i] = a.w[i] << b;
    }
    return rv;
}

static really_really_inline
m128 rshift_m128(m128 a, unsigned b) {
    m128 rv = zeroes128();
    if (b < 32) {
        for (int i = 0; i < 4; i++)
            rv.w[i] = a.w[i] >> b;
    }
    return rv;
}

static really_really_inline
m128 lshift64_m128(m128 a, unsigned b) {
    m128 rv;
    rv.d[0] = a.d[0] << b;
    rv.d[1] = a.d[1] << b;
    return rv;
}

static really_really_inline
m128 rshift64_m128(m128 a, unsigned b) {
    m128 rv;
    rv.d[0] = a.d[0] >> b;
    rv.d[1] = a.d[1] >> b;
    return rv;
}

static really_inline m128 eq128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = (a.b[i] == b.b[i]) ? 0xFF : 0;
    return rv;
}

static really_inline m128 eq64_m128(m128 a, m128 b) {
    m128 rv;
    rv.d[0] = (a.d[0] == b.d[0]) ? ~(u64a)0 : 0;
    rv.d[1] = (a.d[1] == b.d[1]) ? ~(u64a)0 : 0;
    return rv;
}

static really_inline u32 movemask128(m128 a) {
    u32 mask = 0;
    for (int i = 0; i < 16; i++)
        mask |= ((a.b[i] >> 7) << i);
    return mask;
}

static really_inline m128 set1_16x8(u8 c) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = c;
    return rv;
}

static really_inline m128 set1_4x32(u32 c) {
    m128 rv;
    for (int i = 0; i < 4; i++)
        rv.w[i] = c;
    return rv;
}

static really_inline m128 set1_2x64(u64a c) {
    m128 rv;
    rv.d[0] = c;
    rv.d[1] = c;
    return rv;
}

#define insert32_m128(in, val, imm) do { (in).w[(imm)] = (val); } while(0)

static really_inline u32 movd(const m128 in) {
    return in.w[0];
}

static really_inline u64a movq(const m128 in) {
    return in.d[0];
}

static really_inline
m128 load_m128_from_u64a(const u64a *p) {
    m128 rv = zeroes128();
    rv.d[0] = *p;
    return rv;
}

static really_inline u32 extract32from128(const m128 in, unsigned imm) {
    switch (imm) {
    case 0: return in.w[0];
    case 1: return in.w[1];
    case 2: return in.w[2];
    case 3: return in.w[3];
    default: return 0;
    }
}

static really_inline u64a extract64from128(const m128 in, unsigned imm) {
    switch (imm) {
    case 0: return in.d[0];
    case 1: return in.d[1];
    default: return 0;
    }
}

static really_inline m128 low64from128(const m128 in) {
    m128 rv = zeroes128();
    rv.d[0] = in.d[0];
    return rv;
}

static really_inline m128 high64from128(const m128 in) {
    m128 rv = zeroes128();
    rv.d[0] = in.d[1];
    return rv;
}

static really_inline m128 add128(m128 a, m128 b) {
    return add_2x64(a, b);
}

static really_inline m128 and128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = a.b[i] & b.b[i];
    return rv;
}

static really_inline m128 xor128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = a.b[i] ^ b.b[i];
    return rv;
}

static really_inline m128 or128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = a.b[i] | b.b[i];
    return rv;
}

static really_inline m128 andnot128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = (~a.b[i]) & b.b[i];
    return rv;
}

// aligned load
static really_inline m128 load128(const void *ptr) {
    assert(ISALIGNED_N(ptr, alignof(m128)));
    m128 rv;
    memcpy(&rv, ptr, sizeof(rv));
    return rv;
}

// aligned store
static really_inline void store128(void *ptr, m128 a) {
    assert(ISALIGNED_N(ptr, alignof(m128)));
    memcpy(ptr, &a, sizeof(a));
}

// unaligned load
static really_inline m128 loadu128(const void *ptr) {
    m128 rv;
    memcpy(&rv, ptr, sizeof(rv));
    return rv;
}

// unaligned store
static really_inline void storeu128(void *ptr, m128 a) {
    memcpy(ptr, &a, sizeof(a));
}

// packed unaligned store of first N bytes
static really_inline
void storebytes128(void *ptr, m128 a, unsigned int n) {
    assert(n <= sizeof(a));
    memcpy(ptr, &a, n);
}

// packed unaligned load of first N bytes, pad with zero
static really_inline
m128 loadbytes128(const void *ptr, unsigned int n) {
    m128 a = zeroes128();
    assert(n <= sizeof(a));
    memcpy(&a, ptr, n);
    return a;
}

static really_really_inline
m128 palignr_imm(m128 r, m128 l, int offset) {
    m128 rv;
    if (offset == 0) return l;
    if (offset == 16) return r;
    if (offset < 0 || offset > 16) return zeroes128();
    for (int i = 0; i < 16; i++) {
        int src = i + offset;
        rv.b[i] = (src < 16) ? l.b[src] : r.b[src - 16];
    }
    return rv;
}

static really_really_inline
m128 palignr(m128 r, m128 l, int offset) {
    return palignr_imm(r, l, offset);
}

static really_really_inline
m128 rshiftbyte_m128(m128 a, unsigned b) {
    if (b == 0) return a;
    if (b >= 16) return zeroes128();
    return palignr(zeroes128(), a, b);
}

static really_really_inline
m128 lshiftbyte_m128(m128 a, unsigned b) {
    if (b == 0) return a;
    if (b >= 16) return zeroes128();
    return palignr(a, zeroes128(), 16 - b);
}

static really_inline
m128 variable_byte_shift_m128(m128 in, s32 amount) {
    assert(amount >= -16 && amount <= 16);
    if (amount < 0) {
        return palignr_imm(zeroes128(), in, -amount);
    } else {
        return palignr_imm(in, zeroes128(), 16 - amount);
    }
}

static really_inline
m128 mask1bit128(unsigned int n) {
    assert(n < sizeof(m128) * 8);
    m128 rv = zeroes128();
    rv.b[n / 8] = (u8)(1U << (n % 8));
    return rv;
}

static really_inline
void setbit128(m128 *ptr, unsigned int n) {
    *ptr = or128(mask1bit128(n), *ptr);
}

static really_inline
void clearbit128(m128 *ptr, unsigned int n) {
    *ptr = andnot128(mask1bit128(n), *ptr);
}

static really_inline
char testbit128(m128 val, unsigned int n) {
    const m128 mask = mask1bit128(n);
    return isnonzero128(and128(mask, val));
}

static really_inline
m128 pshufb_m128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++) {
        u8 idx = b.b[i];
        if (idx & 0x80) {
            rv.b[i] = 0;
        } else {
            rv.b[i] = a.b[idx & 0xf];
        }
    }
    return rv;
}

static really_inline
m128 max_u8_m128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = (a.b[i] > b.b[i]) ? a.b[i] : b.b[i];
    return rv;
}

static really_inline
m128 min_u8_m128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = (a.b[i] < b.b[i]) ? a.b[i] : b.b[i];
    return rv;
}

static really_inline
m128 sadd_u8_m128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++) {
        u16 sum = (u16)a.b[i] + (u16)b.b[i];
        rv.b[i] = (sum > 255) ? 255 : (u8)sum;
    }
    return rv;
}

static really_inline
m128 sub_u8_m128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++)
        rv.b[i] = a.b[i] - b.b[i];
    return rv;
}

static really_inline
m128 set4x32(u32 x3, u32 x2, u32 x1, u32 x0) {
    m128 rv;
    rv.w[0] = x0;
    rv.w[1] = x1;
    rv.w[2] = x2;
    rv.w[3] = x3;
    return rv;
}

static really_inline
m128 set2x64(u64a hi, u64a lo) {
    m128 rv;
    rv.d[0] = lo;
    rv.d[1] = hi;
    return rv;
}

static really_inline
m128 widenlo128(m128 x) {
    m128 rv = zeroes128();
    rv.w[0] = x.w[0];
    rv.w[1] = x.w[1];
    return rv;
}

static really_inline
m128 widenhi128(m128 x) {
    m128 rv = zeroes128();
    rv.w[0] = x.w[2];
    rv.w[1] = x.w[3];
    return rv;
}

#endif // ARCH_RISCV64_SIMD_UTILS_H
