/*
 * Copyright (c) 2015-2017, Intel Corporation
 * Copyright (c) 2020-2021, VectorCamp PC
 * Copyright (c) 2024, Kylin Software Co., Ltd.
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

#ifndef SIMD_IMPL_SW64_HPP
#define SIMD_IMPL_SW64_HPP

#include <cstdint>
#include <cstring>

#include "ue2common.h"
#include "util/supervector/supervector.hpp"

// 128-bit scalar implementation for SW64

template<>
inline SuperVector<16>::SuperVector(typename base_type::type const v) {
    u.v128[0] = v;
}

template<> template<> inline SuperVector<16>::SuperVector(int8_t const other) {
    for (int i = 0; i < 16; i++) u.s8[i] = other;
}
template<> template<> inline SuperVector<16>::SuperVector(uint8_t const other) {
    memset(u.u8, other, 16);
}
template<> template<> inline SuperVector<16>::SuperVector(int16_t const other) {
    for (int i = 0; i < 8; i++) u.s16[i] = other;
}
template<> template<> inline SuperVector<16>::SuperVector(uint16_t const other) {
    for (int i = 0; i < 8; i++) u.u16[i] = other;
}
template<> template<> inline SuperVector<16>::SuperVector(int32_t const other) {
    for (int i = 0; i < 4; i++) u.s32[i] = other;
}
template<> template<> inline SuperVector<16>::SuperVector(uint32_t const other) {
    for (int i = 0; i < 4; i++) u.u32[i] = other;
}
template<> template<> inline SuperVector<16>::SuperVector(int64_t const other) {
    u.s64[0] = other; u.s64[1] = other;
}
template<> template<> inline SuperVector<16>::SuperVector(uint64_t const other) {
    u.u64[0] = other; u.u64[1] = other;
}

template<> inline SuperVector<16> SuperVector<16>::Ones(void) {
    SuperVector<16> rv;
    memset(rv.u.u8, 0xFF, 16);
    return rv;
}
template<> inline SuperVector<16> SuperVector<16>::Zeroes(void) {
    SuperVector<16> rv;
    memset(rv.u.u8, 0, 16);
    return rv;
}

template<> inline void SuperVector<16>::operator=(SuperVector<16> const &other) {
    u.v128[0] = other.u.v128[0];
}

#define SV_BINOP(OP, FIELD) \
template<> inline SuperVector<16> SuperVector<16>::operator OP(SuperVector<16> const &b) const { \
    SuperVector<16> rv; \
    for (int i = 0; i < 2; i++) rv.u.FIELD[i] = this->u.FIELD[i] OP b.u.FIELD[i]; \
    return rv; \
}

SV_BINOP(&, u64)
SV_BINOP(|, u64)
SV_BINOP(^, u64)
#undef SV_BINOP

template<> inline SuperVector<16> SuperVector<16>::operator!() const {
    SuperVector<16> rv;
    for (int i = 0; i < 2; i++) rv.u.u64[i] = ~u.u64[i];
    return rv;
}

template<> inline SuperVector<16> SuperVector<16>::opandnot(SuperVector<16> const &b) const {
    SuperVector<16> rv;
    for (int i = 0; i < 2; i++) rv.u.u64[i] = ~u.u64[i] & b.u.u64[i];
    return rv;
}

template<> inline SuperVector<16> SuperVector<16>::operator==(SuperVector<16> const &b) const {
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) rv.u.u8[i] = (u.u8[i] == b.u.u8[i]) ? 0xFF : 0x00;
    return rv;
}
template<> inline SuperVector<16> SuperVector<16>::operator!=(SuperVector<16> const &b) const {
    return !(*this == b);
}
template<> inline SuperVector<16> SuperVector<16>::operator>(SuperVector<16> const &b) const {
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) rv.u.u8[i] = (u.u8[i] > b.u.u8[i]) ? 0xFF : 0x00;
    return rv;
}
template<> inline SuperVector<16> SuperVector<16>::operator>=(SuperVector<16> const &b) const {
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) rv.u.u8[i] = (u.u8[i] >= b.u.u8[i]) ? 0xFF : 0x00;
    return rv;
}
template<> inline SuperVector<16> SuperVector<16>::operator<(SuperVector<16> const &b) const {
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) rv.u.u8[i] = (u.u8[i] < b.u.u8[i]) ? 0xFF : 0x00;
    return rv;
}
template<> inline SuperVector<16> SuperVector<16>::operator<=(SuperVector<16> const &b) const {
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) rv.u.u8[i] = (u.u8[i] <= b.u.u8[i]) ? 0xFF : 0x00;
    return rv;
}

template<> inline SuperVector<16> SuperVector<16>::eq(SuperVector<16> const &b) const {
    return *this == b;
}

template<> inline typename SuperVector<16>::comparemask_type SuperVector<16>::comparemask(void) const {
    u64a mask = 0;
    for (int i = 0; i < 16; i++) if (u.u8[i] & 0x80) mask |= (1ULL << i);
    return static_cast<comparemask_type>(mask);
}

template<> inline typename SuperVector<16>::comparemask_type SuperVector<16>::eqmask(SuperVector<16> const b) const {
    return eq(b).comparemask();
}

template<> inline u32 SuperVector<16>::mask_width() { return 1; }

template<> inline typename SuperVector<16>::comparemask_type SuperVector<16>::iteration_mask(typename SuperVector<16>::comparemask_type mask) {
    return mask;
}

// Immediate shift templates
#define SV_VSHL_IMM(BITS, FIELD, COUNT) \
template<> template<uint8_t N> inline SuperVector<16> SuperVector<16>::vshl_##BITS##_imm() const { \
    SuperVector<16> rv; \
    for (int i = 0; i < 16/(BITS/8); i++) rv.u.FIELD[i] = u.FIELD[i] << N; \
    return rv; \
}
SV_VSHL_IMM(8, u8, 1)
SV_VSHL_IMM(16, u16, 2)
SV_VSHL_IMM(32, u32, 4)
SV_VSHL_IMM(64, u64, 8)
#undef SV_VSHL_IMM

template<> template<uint8_t N> inline SuperVector<16> SuperVector<16>::vshl_128_imm() const {
    SuperVector<16> rv = Zeroes();
    if (N < 16) for (int i = 15; i >= N; i--) rv.u.u8[i] = u.u8[i - N];
    return rv;
}
template<> template<uint8_t N> inline SuperVector<16> SuperVector<16>::vshl_imm() const { return vshl_128_imm<N>(); }

#define SV_VSHR_IMM(BITS, FIELD) \
template<> template<uint8_t N> inline SuperVector<16> SuperVector<16>::vshr_##BITS##_imm() const { \
    SuperVector<16> rv; \
    for (int i = 0; i < 16/(BITS/8); i++) rv.u.FIELD[i] = u.FIELD[i] >> N; \
    return rv; \
}
SV_VSHR_IMM(8, u8)
SV_VSHR_IMM(16, u16)
SV_VSHR_IMM(32, u32)
SV_VSHR_IMM(64, u64)
#undef SV_VSHR_IMM

template<> template<uint8_t N> inline SuperVector<16> SuperVector<16>::vshr_128_imm() const {
    SuperVector<16> rv = Zeroes();
    if (N < 16) for (int i = 0; i < 16 - N; i++) rv.u.u8[i] = u.u8[i + N];
    return rv;
}
template<> template<uint8_t N> inline SuperVector<16> SuperVector<16>::vshr_imm() const { return vshr_128_imm<N>(); }

// Variable shifts
#define SV_VSHL_VAR(BITS, FIELD) \
template<> inline SuperVector<16> SuperVector<16>::vshl_##BITS(uint8_t const N) const { \
    SuperVector<16> rv; \
    if (N == 0) { rv = *this; return rv; } \
    for (int i = 0; i < 16/(BITS/8); i++) rv.u.FIELD[i] = u.FIELD[i] << N; \
    return rv; \
}
SV_VSHL_VAR(8, u8)
SV_VSHL_VAR(16, u16)
SV_VSHL_VAR(32, u32)
SV_VSHL_VAR(64, u64)
#undef SV_VSHL_VAR

template<> inline SuperVector<16> SuperVector<16>::vshl_128(uint8_t const N) const {
    SuperVector<16> rv = Zeroes();
    if (N < 16) for (int i = 15; i >= N; i--) rv.u.u8[i] = u.u8[i - N];
    return rv;
}
template<> inline SuperVector<16> SuperVector<16>::vshl(uint8_t const N) const { return vshl_128(N); }

#define SV_VSHR_VAR(BITS, FIELD) \
template<> inline SuperVector<16> SuperVector<16>::vshr_##BITS(uint8_t const N) const { \
    SuperVector<16> rv; \
    if (N == 0) { rv = *this; return rv; } \
    for (int i = 0; i < 16/(BITS/8); i++) rv.u.FIELD[i] = u.FIELD[i] >> N; \
    return rv; \
}
SV_VSHR_VAR(8, u8)
SV_VSHR_VAR(16, u16)
SV_VSHR_VAR(32, u32)
SV_VSHR_VAR(64, u64)
#undef SV_VSHR_VAR

template<> inline SuperVector<16> SuperVector<16>::vshr_128(uint8_t const N) const {
    SuperVector<16> rv = Zeroes();
    if (N < 16) for (int i = 0; i < 16 - N; i++) rv.u.u8[i] = u.u8[i + N];
    return rv;
}
template<> inline SuperVector<16> SuperVector<16>::vshr(uint8_t const N) const { return vshr_128(N); }

template<> inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const { return vshr_128(N); }
template<> inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const { return vshl_128(N); }

template<> inline SuperVector<16> SuperVector<16>::Ones_vshr(uint8_t const N) {
    SuperVector<16> rv = Ones();
    return rv.vshr_128(N);
}
template<> inline SuperVector<16> SuperVector<16>::Ones_vshl(uint8_t const N) {
    SuperVector<16> rv = Ones();
    return rv.vshl_128(N);
}

template<> inline SuperVector<16> SuperVector<16>::loadu(void const *ptr) {
    SuperVector<16> rv;
    memcpy(rv.u.u8, ptr, 16);
    return rv;
}
template<> inline SuperVector<16> SuperVector<16>::load(void const *ptr) {
    assert(ISALIGNED_N(ptr, 16));
    return loadu(ptr);
}
template<> inline SuperVector<16> SuperVector<16>::loadu_maskz(void const *ptr, uint8_t const len) {
    SuperVector<16> rv = Zeroes();
    assert(len <= 16);
    memcpy(rv.u.u8, ptr, len);
    return rv;
}

template<> inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset) {
    SuperVector<16> rv = Zeroes();
    offset &= 0xff;
    if (offset < 16) {
        for (int i = 0; i < 16; i++) {
            int src = i + offset;
            rv.u.u8[i] = (src < 16) ? other.u.u8[src] : u.u8[src - 16];
        }
    } else if (offset < 32) {
        int off = offset - 16;
        for (int i = 0; i < 16; i++) { int src = i + off; if (src < 16) rv.u.u8[i] = u.u8[src]; }
    }
    return rv;
}

template<> template<bool restore_signs> inline SuperVector<16> SuperVector<16>::pshufb(SuperVector<16> b) {
    SuperVector<16> rv = Zeroes();
    if (restore_signs) {
        for (int i = 0; i < 16; i++) {
            u8 idx = b.u.u8[i];
            if (!(idx & 0x80)) rv.u.u8[i] = u.u8[idx & 0x0F];
        }
    } else {
        for (int i = 0; i < 16; i++) {
            u8 idx = b.u.u8[i] & 0x0F;
            rv.u.u8[i] = u.u8[idx];
        }
    }
    return rv;
}

template<> inline SuperVector<16> SuperVector<16>::pshufb_maskz(SuperVector<16> b, uint8_t const len) {
    SuperVector<16> rv = Zeroes();
    for (int i = 0; i < len && i < 16; i++) {
        u8 idx = b.u.u8[i];
        if (!(idx & 0x80)) rv.u.u8[i] = u.u8[idx & 0x0F];
    }
    return rv;
}

// dup methods
template<> inline SuperVector<16> SuperVector<16>::dup_s8(int8_t const other) { return SuperVector<16>(other); }
template<> inline SuperVector<16> SuperVector<16>::dup_u8(uint8_t const other) { return SuperVector<16>(other); }
template<> inline SuperVector<16> SuperVector<16>::dup_s16(int16_t const other) { return SuperVector<16>(other); }
template<> inline SuperVector<16> SuperVector<16>::dup_u16(uint16_t const other) { return SuperVector<16>(other); }
template<> inline SuperVector<16> SuperVector<16>::dup_s32(int32_t const other) { return SuperVector<16>(other); }
template<> inline SuperVector<16> SuperVector<16>::dup_u32(uint32_t const other) { return SuperVector<16>(other); }
template<> inline SuperVector<16> SuperVector<16>::dup_s64(int64_t const other) { return SuperVector<16>(other); }
template<> inline SuperVector<16> SuperVector<16>::dup_u64(uint64_t const other) { return SuperVector<16>(other); }

// print methods (no-op in release)
#ifdef DEBUG
template<> inline void SuperVector<16>::print8(const char *label) const {
    DEBUG_PRINTF("%12s: ", label);
    for (int i = 15; i >= 0; i--) printf("%02x ", u.u8[i]);
    printf("\n");
}
template<> inline void SuperVector<16>::print16(const char *label) const {
    DEBUG_PRINTF("%12s: ", label);
    for (int i = 7; i >= 0; i--) printf("%04x ", u.u16[i]);
    printf("\n");
}
template<> inline void SuperVector<16>::print32(const char *label) const {
    DEBUG_PRINTF("%12s: ", label);
    for (int i = 3; i >= 0; i--) printf("%08x ", u.u32[i]);
    printf("\n");
}
template<> inline void SuperVector<16>::print64(const char *label) const {
    DEBUG_PRINTF("%12s: ", label);
    for (int i = 1; i >= 0; i--) printf("%016llx ", (unsigned long long)u.u64[i]);
    printf("\n");
}
#else
template<> inline void SuperVector<16>::print8(const char *label) const {}
template<> inline void SuperVector<16>::print16(const char *label) const {}
template<> inline void SuperVector<16>::print32(const char *label) const {}
template<> inline void SuperVector<16>::print64(const char *label) const {}
#endif

#endif // SIMD_IMPL_SW64_HPP
