/*
 * Copyright (c) 2015-2020, Intel Corporation
 * Copyright (c) 2024, VectorCamp PC
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
 * \brief SIMD 128-bit scalar primitives for SW64 (Sunway).
 */

#ifndef SIMD_UTILS_SW64_H
#define SIMD_UTILS_SW64_H

#include "ue2common.h"
#include "util/simd_types.h"
#include "util/unaligned.h"
#include "util/arch.h"

#include <string.h>

static really_inline m128 ones128(void) {
    m128 rv; memset(&rv, 0xFF, sizeof(rv)); return rv;
}
static really_inline m128 zeroes128(void) {
    m128 rv; memset(&rv, 0, sizeof(rv)); return rv;
}
static really_inline m128 not128(m128 a) {
    m128 rv; rv.m64[0] = ~a.m64[0]; rv.m64[1] = ~a.m64[1]; return rv;
}
static really_inline int diff128(m128 a, m128 b) {
    return !!(a.m64[0] != b.m64[0] || a.m64[1] != b.m64[1]);
}
static really_inline int isnonzero128(m128 a) {
    return !!(a.m64[0] | a.m64[1]);
}
static really_inline u32 diffrich128(m128 a, m128 b) {
    u32 mask = 0;
    for (int i = 0; i < 4; i++) if (a.m32[i] != b.m32[i]) mask |= (1u << i);
    return mask;
}
static really_inline u32 diffrich64_128(m128 a, m128 b) {
    u32 d = diffrich128(a, b); return (d | (d >> 1)) & 0x5;
}
static really_inline m128 eq128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 16; i++) rv.m8[i] = (a.m8[i] == b.m8[i]) ? 0xFF : 0x00;
    return rv;
}
static really_inline m128 eq64_m128(m128 a, m128 b) {
    m128 rv;
    for (int i = 0; i < 2; i++) rv.m64[i] = (a.m64[i] == b.m64[i]) ? ~(u64a)0 : 0;
    return rv;
}
static really_inline u32 movemask128(m128 a) {
    u32 mask = 0;
    for (int i = 0; i < 16; i++) if (a.m8[i] & 0x80) mask |= (1u << i);
    return mask;
}
static really_really_inline m128 lshift64_m128(m128 a, unsigned b) {
    assert(b <= 63); m128 rv; rv.m64[0] = a.m64[0] << b; rv.m64[1] = a.m64[1] << b; return rv;
}
static really_really_inline m128 rshift64_m128(m128 a, unsigned b) {
    assert(b <= 63); if (b == 0) return a; m128 rv; rv.m64[0] = a.m64[0] >> b; rv.m64[1] = a.m64[1] >> b; return rv;
}
static really_inline m128 set1_16x8(u8 c) {
    m128 rv; memset(&rv, c, sizeof(rv)); return rv;
}
static really_inline m128 set1_4x32(u32 c) {
    m128 rv; rv.m32[0] = c; rv.m32[1] = c; rv.m32[2] = c; rv.m32[3] = c; return rv;
}
static really_inline m128 set1_2x64(u64a c) {
    m128 rv; rv.m64[0] = c; rv.m64[1] = c; return rv;
}
static really_inline m128 set64x2(u64a hi, u64a lo) {
    m128 rv; rv.m64[0] = lo; rv.m64[1] = hi; return rv;
}
static really_inline m128 set32x4(u32 i3, u32 i2, u32 i1, u32 i0) {
    m128 rv; rv.m32[0] = i0; rv.m32[1] = i1; rv.m32[2] = i2; rv.m32[3] = i3; return rv;
}
static really_inline m128 set4x32(u32 x3, u32 x2, u32 x1, u32 x0) {
    m128 rv; rv.m32[0] = x0; rv.m32[1] = x1; rv.m32[2] = x2; rv.m32[3] = x3; return rv;
}
static really_inline m128 set2x64(u64a hi, u64a lo) {
    m128 rv; rv.m64[0] = lo; rv.m64[1] = hi; return rv;
}
static really_inline u32 movd(const m128 in) { return in.m32[0]; }
static really_inline u64a movq(const m128 in) { return in.m64[0]; }
static really_inline u64a low64from128(m128 a) { return a.m64[0]; }
static really_inline u64a high64from128(m128 a) { return a.m64[1]; }
static really_inline u32 extract32from128(m128 a, int imm) { return a.m32[imm & 0x3]; }
static really_inline u64a extract64from128(m128 a, int imm) { return a.m64[imm & 0x1]; }
static really_inline m128 load_m128_from_u64a(const u64a *p) {
    m128 rv = zeroes128(); rv.m64[0] = *p; return rv;
}
static really_inline m128 and128(m128 a, m128 b) {
    m128 rv; rv.m64[0] = a.m64[0] & b.m64[0]; rv.m64[1] = a.m64[1] & b.m64[1]; return rv;
}
static really_inline m128 or128(m128 a, m128 b) {
    m128 rv; rv.m64[0] = a.m64[0] | b.m64[0]; rv.m64[1] = a.m64[1] | b.m64[1]; return rv;
}
static really_inline m128 xor128(m128 a, m128 b) {
    m128 rv; rv.m64[0] = a.m64[0] ^ b.m64[0]; rv.m64[1] = a.m64[1] ^ b.m64[1]; return rv;
}
static really_inline m128 andnot128(m128 a, m128 b) {
    m128 rv; rv.m64[0] = ~a.m64[0] & b.m64[0]; rv.m64[1] = ~a.m64[1] & b.m64[1]; return rv;
}
static really_inline m128 add128(m128 a, m128 b) {
    m128 rv; rv.m64[0] = a.m64[0] + b.m64[0]; rv.m64[1] = a.m64[1] + b.m64[1]; return rv;
}
static really_inline m128 sub_2x64(m128 a, m128 b) {
    m128 rv; rv.m64[0] = a.m64[0] - b.m64[0]; rv.m64[1] = a.m64[1] - b.m64[1]; return rv;
}
static really_inline m128 load128(const void *ptr) {
    assert(ISALIGNED_N(ptr, alignof(m128))); m128 rv; memcpy(&rv, ptr, sizeof(rv)); return rv;
}
static really_inline void store128(void *ptr, m128 a) {
    assert(ISALIGNED_N(ptr, alignof(m128))); memcpy(ptr, &a, sizeof(a));
}
static really_inline m128 loadu128(const void *ptr) {
    m128 rv; memcpy(&rv, ptr, sizeof(rv)); return rv;
}
static really_inline void storeu128(void *ptr, m128 a) {
    memcpy(ptr, &a, sizeof(a));
}
static really_inline void storebytes128(void *ptr, m128 a, unsigned int n) {
    assert(n <= sizeof(a)); memcpy(ptr, &a, n);
}
static really_inline m128 loadbytes128(const void *ptr, unsigned int n) {
    m128 a = zeroes128(); assert(n <= sizeof(a)); memcpy(&a, ptr, n); return a;
}
static really_inline m128 palignr_imm(m128 r, m128 l, int offset) {
    m128 rv = zeroes128(); offset = offset & 0xff;
    if (offset < 16) {
        for (int i = 0; i < 16; i++) { int src = i + offset; rv.m8[i] = (src < 16) ? l.m8[src] : r.m8[src - 16]; }
    } else if (offset < 32) {
        int off = offset - 16;
        for (int i = 0; i < 16; i++) { int src = i + off; if (src < 16) rv.m8[i] = r.m8[src]; }
    }
    return rv;
}
static really_inline m128 palignr(m128 r, m128 l, int offset) { return palignr_imm(r, l, offset); }
static really_inline m128 rshiftbyte_m128(m128 a, unsigned b) { if (b == 0) return a; return palignr(zeroes128(), a, b); }
static really_inline m128 lshiftbyte_m128(m128 a, unsigned b) { if (b == 0) return a; return palignr(a, zeroes128(), 16 - b); }
static really_inline m128 mask1bit128(unsigned int n) {
    assert(n < sizeof(m128) * 8);
    m128 onebit = zeroes128();
    onebit.m8[0] = 1;
    m128 mask = lshiftbyte_m128(onebit, n / 8);
    return lshift64_m128(mask, n % 8);
}
static really_inline void setbit128(m128 *ptr, unsigned int n) { *ptr = or128(mask1bit128(n), *ptr); }
static really_inline void clearbit128(m128 *ptr, unsigned int n) { *ptr = andnot128(mask1bit128(n), *ptr); }
static really_inline char testbit128(m128 val, unsigned int n) {
    return isnonzero128(and128(mask1bit128(n), val));
}
static really_inline m128 variable_byte_shift_m128(m128 in, s32 amount) {
    assert(amount >= -16 && amount <= 16);
    if (amount <= 0) return palignr_imm(zeroes128(), in, -amount);
    else return palignr_imm(in, zeroes128(), 16 - amount);
}
static really_inline m128 pshufb_m128(m128 a, m128 b) {
    m128 rv = zeroes128();
    for (int i = 0; i < 16; i++) { u8 idx = b.m8[i]; if (!(idx & 0x80)) rv.m8[i] = a.m8[idx & 0x0F]; }
    return rv;
}
static really_inline m128 max_u8_m128(m128 a, m128 b) {
    m128 rv; for (int i = 0; i < 16; i++) rv.m8[i] = (a.m8[i] > b.m8[i]) ? a.m8[i] : b.m8[i]; return rv;
}
static really_inline m128 min_u8_m128(m128 a, m128 b) {
    m128 rv; for (int i = 0; i < 16; i++) rv.m8[i] = (a.m8[i] < b.m8[i]) ? a.m8[i] : b.m8[i]; return rv;
}
static really_inline m128 sadd_u8_m128(m128 a, m128 b) {
    m128 rv; for (int i = 0; i < 16; i++) { u16 s = (u16)a.m8[i] + (u16)b.m8[i]; rv.m8[i] = (s > 0xFF) ? 0xFF : (u8)s; } return rv;
}
static really_inline m128 sub_u8_m128(m128 a, m128 b) {
    m128 rv; for (int i = 0; i < 16; i++) { int d = (int)a.m8[i] - (int)b.m8[i]; rv.m8[i] = (d < 0) ? 0 : (u8)d; } return rv;
}
static really_inline m128 widenlo128(m128 x) {
    m128 rv = zeroes128(); for (int i = 0; i < 8; i++) rv.m16[i] = (u16)x.m8[i]; return rv;
}
static really_inline m128 widenhi128(m128 x) {
    m128 rv = zeroes128(); for (int i = 0; i < 8; i++) rv.m16[i] = (u16)x.m8[i + 8]; return rv;
}

#endif // SIMD_UTILS_SW64_H
