/*
 * Copyright (c) 2015-2017, Intel Corporation
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

#ifndef SIMD_IMPL_HPP
#define SIMD_IMPL_HPP

#include <cstdint>
#include <cstring>

#include "ue2common.h"
#include "util/supervector/supervector.hpp"

// 128-bit RISC-V implementation using scalar operations on m128 union.
// m128 union members: b=bytes(16), h=halfwords(8), w=words(4), d=doublewords(2).

// Constructors from scalar types

template<>
really_inline SuperVector<16>::SuperVector(m128 other)
{
    u.v128[0] = other;
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int8_t const other)
{
    for (int i = 0; i < 16; i++) u.v128[0].b[i] = (u8)other;
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint8_t const other)
{
    for (int i = 0; i < 16; i++) u.v128[0].b[i] = other;
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int16_t const other)
{
    for (int i = 0; i < 8; i++) u.v128[0].h[i] = (u16)other;
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint16_t const other)
{
    for (int i = 0; i < 8; i++) u.v128[0].h[i] = other;
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int32_t const other)
{
    for (int i = 0; i < 4; i++) u.v128[0].w[i] = (u32)other;
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint32_t const other)
{
    for (int i = 0; i < 4; i++) u.v128[0].w[i] = other;
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(int64_t const other)
{
    for (int i = 0; i < 2; i++) u.v128[0].d[i] = (u64a)other;
}

template<>
template<>
really_inline SuperVector<16>::SuperVector(uint64_t const other)
{
    for (int i = 0; i < 2; i++) u.v128[0].d[i] = other;
}

// Constants
template<>
really_inline SuperVector<16> SuperVector<16>::Ones(void)
{
    return SuperVector<16>((uint8_t)0xFF);
}

template<>
really_inline SuperVector<16> SuperVector<16>::Zeroes(void)
{
    return SuperVector<16>((uint8_t)0);
}

// Methods

template <>
really_inline void SuperVector<16>::operator=(SuperVector<16> const &other)
{
    u.v128[0] = other.u.v128[0];
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator&(SuperVector<16> const &b) const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++)
        rv.u.v128[0].b[i] = u.v128[0].b[i] & b.u.v128[0].b[i];
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator|(SuperVector<16> const &b) const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++)
        rv.u.v128[0].b[i] = u.v128[0].b[i] | b.u.v128[0].b[i];
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator^(SuperVector<16> const &b) const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++)
        rv.u.v128[0].b[i] = u.v128[0].b[i] ^ b.u.v128[0].b[i];
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator!() const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++)
        rv.u.v128[0].b[i] = ~u.v128[0].b[i];
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::opandnot(SuperVector<16> const &b) const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++)
        rv.u.v128[0].b[i] = (~u.v128[0].b[i]) & b.u.v128[0].b[i];
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator==(SuperVector<16> const &b) const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++)
        rv.u.v128[0].b[i] = (u.v128[0].b[i] == b.u.v128[0].b[i]) ? 0xFF : 0;
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator!=(SuperVector<16> const &b) const
{
    return !(*this == b);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>(SuperVector<16> const &b) const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) {
        s8 a_val = (s8)u.v128[0].b[i];
        s8 b_val = (s8)b.u.v128[0].b[i];
        rv.u.v128[0].b[i] = (a_val > b_val) ? 0xFF : 0;
    }
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>=(SuperVector<16> const &b) const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) {
        s8 a_val = (s8)u.v128[0].b[i];
        s8 b_val = (s8)b.u.v128[0].b[i];
        rv.u.v128[0].b[i] = (a_val >= b_val) ? 0xFF : 0;
    }
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<(SuperVector<16> const &b) const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) {
        s8 a_val = (s8)u.v128[0].b[i];
        s8 b_val = (s8)b.u.v128[0].b[i];
        rv.u.v128[0].b[i] = (a_val < b_val) ? 0xFF : 0;
    }
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<=(SuperVector<16> const &b) const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) {
        s8 a_val = (s8)u.v128[0].b[i];
        s8 b_val = (s8)b.u.v128[0].b[i];
        rv.u.v128[0].b[i] = (a_val <= b_val) ? 0xFF : 0;
    }
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::eq(SuperVector<16> const &b) const
{
    return (*this == b);
}

template <>
really_inline typename SuperVector<16>::comparemask_type
SuperVector<16>::comparemask(void) const {
    u64a mask = 0;
    for (int i = 0; i < 16; i++)
        mask |= ((u64a)((u.v128[0].b[i] >> 7) & 1) << i);
    return static_cast<typename SuperVector<16>::comparemask_type>(mask);
}

template <>
really_inline typename SuperVector<16>::comparemask_type
SuperVector<16>::eqmask(SuperVector<16> const b) const {
    return eq(b).comparemask();
}

template <> really_inline u32 SuperVector<16>::mask_width() { return 1; }

template <>
really_inline typename SuperVector<16>::comparemask_type
SuperVector<16>::iteration_mask(
    typename SuperVector<16>::comparemask_type mask) {
    return mask;
}

// Shift operations (template versions for immediate N)

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_8_imm() const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) rv.u.v128[0].b[i] = u.v128[0].b[i] << N;
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_16_imm() const
{
    SuperVector<16> rv;
    for (int i = 0; i < 8; i++) rv.u.v128[0].h[i] = u.v128[0].h[i] << N;
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_32_imm() const
{
    SuperVector<16> rv;
    for (int i = 0; i < 4; i++) rv.u.v128[0].w[i] = u.v128[0].w[i] << N;
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_64_imm() const
{
    SuperVector<16> rv;
    for (int i = 0; i < 2; i++) rv.u.v128[0].d[i] = u.v128[0].d[i] << N;
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_128_imm() const
{
    SuperVector<16> rv = Zeroes();
    for (int i = N; i < 16; i++)
        rv.u.v128[0].b[i] = u.v128[0].b[i - N];
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshl_imm() const
{
    return vshl_128_imm<N>();
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_8_imm() const
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) rv.u.v128[0].b[i] = u.v128[0].b[i] >> N;
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_16_imm() const
{
    SuperVector<16> rv;
    for (int i = 0; i < 8; i++) rv.u.v128[0].h[i] = u.v128[0].h[i] >> N;
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_32_imm() const
{
    SuperVector<16> rv;
    for (int i = 0; i < 4; i++) rv.u.v128[0].w[i] = u.v128[0].w[i] >> N;
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_64_imm() const
{
    SuperVector<16> rv;
    for (int i = 0; i < 2; i++) rv.u.v128[0].d[i] = u.v128[0].d[i] >> N;
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_128_imm() const
{
    SuperVector<16> rv = Zeroes();
    for (int i = 0; i < 16 - N; i++)
        rv.u.v128[0].b[i] = u.v128[0].b[i + N];
    return rv;
}

template<>
template<uint8_t N>
really_inline SuperVector<16> SuperVector<16>::vshr_imm() const
{
    return vshr_128_imm<N>();
}

#if !defined(HS_OPTIMIZE)
template SuperVector<16> SuperVector<16>::vshl_8_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshl_16_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_64_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_64_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshl_128_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshl_128_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_8_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_8_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_16_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_64_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_64_imm<4>() const;
template SuperVector<16> SuperVector<16>::vshr_128_imm<1>() const;
template SuperVector<16> SuperVector<16>::vshr_128_imm<4>() const;
#endif

// Dynamic shift operations

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_8(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 8) return Zeroes();
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) rv.u.v128[0].b[i] = u.v128[0].b[i] << N;
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_16(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 16) return Zeroes();
    SuperVector<16> rv;
    for (int i = 0; i < 8; i++) rv.u.v128[0].h[i] = u.v128[0].h[i] << N;
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_32(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 32) return Zeroes();
    SuperVector<16> rv;
    for (int i = 0; i < 4; i++) rv.u.v128[0].w[i] = u.v128[0].w[i] << N;
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_64(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 64) return Zeroes();
    SuperVector<16> rv;
    for (int i = 0; i < 2; i++) rv.u.v128[0].d[i] = u.v128[0].d[i] << N;
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl_128(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 16) return Zeroes();
    SuperVector<16> rv = Zeroes();
    for (int i = N; i < 16; i++)
        rv.u.v128[0].b[i] = u.v128[0].b[i - N];
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshl(uint8_t const N) const
{
    return vshl_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_8(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 8) return Zeroes();
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) rv.u.v128[0].b[i] = u.v128[0].b[i] >> N;
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_16(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 16) return Zeroes();
    SuperVector<16> rv;
    for (int i = 0; i < 8; i++) rv.u.v128[0].h[i] = u.v128[0].h[i] >> N;
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_32(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 32) return Zeroes();
    SuperVector<16> rv;
    for (int i = 0; i < 4; i++) rv.u.v128[0].w[i] = u.v128[0].w[i] >> N;
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_64(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 64) return Zeroes();
    SuperVector<16> rv;
    for (int i = 0; i < 2; i++) rv.u.v128[0].d[i] = u.v128[0].d[i] >> N;
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr_128(uint8_t const N) const
{
    if (N == 0) return *this;
    if (N >= 16) return Zeroes();
    SuperVector<16> rv = Zeroes();
    for (int i = 0; i < 16 - N; i++)
        rv.u.v128[0].b[i] = u.v128[0].b[i + N];
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::vshr(uint8_t const N) const
{
    return vshr_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const
{
    return vshr_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const
{
    return vshl_128(N);
}

template<>
really_inline SuperVector<16> SuperVector<16>::Ones_vshr(uint8_t const N)
{
    return Ones().vshr_128(N);
}

template<>
really_inline SuperVector<16> SuperVector<16>::Ones_vshl(uint8_t const N)
{
    return Ones().vshl_128(N);
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu(void const *ptr)
{
    SuperVector<16> rv;
    memcpy(&rv.u.v128[0], ptr, 16);
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::load(void const *ptr)
{
    assert(ISALIGNED_N(ptr, alignof(SuperVector::size)));
    ptr = vectorscan_assume_aligned(ptr, SuperVector::size);
    SuperVector<16> rv;
    memcpy(&rv.u.v128[0], ptr, 16);
    return rv;
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu_maskz(void const *ptr, uint8_t const len)
{
    SuperVector mask = Ones_vshr(16 - len);
    SuperVector<16> v = loadu(ptr);
    return mask & v;
}

template<>
really_inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset)
{
    if (offset == 0) return other;
    if (offset == 16) return *this;
    if (offset < 0 || offset > 16) return Zeroes();
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) {
        int src = i + offset;
        if (src < 16)
            rv.u.v128[0].b[i] = other.u.v128[0].b[src];
        else
            rv.u.v128[0].b[i] = u.v128[0].b[src - 16];
    }
    return rv;
}

template<>
template<>
really_inline SuperVector<16> SuperVector<16>::pshufb<false>(SuperVector<16> b)
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) {
        u8 idx = b.u.v128[0].b[i];
        rv.u.v128[0].b[i] = u.v128[0].b[idx & 0xf];
    }
    return rv;
}

template<>
template<>
really_inline SuperVector<16> SuperVector<16>::pshufb<true>(SuperVector<16> b)
{
    SuperVector<16> rv;
    for (int i = 0; i < 16; i++) {
        u8 idx = b.u.v128[0].b[i];
        if (idx & 0x80)
            rv.u.v128[0].b[i] = 0;
        else
            rv.u.v128[0].b[i] = u.v128[0].b[idx & 0xf];
    }
    return rv;
}

template<>
really_inline SuperVector<16> SuperVector<16>::pshufb_maskz(SuperVector<16> b, uint8_t const len)
{
    SuperVector mask = Ones_vshr(16 - len);
    return mask & pshufb(b);
}

#endif // SIMD_IMPL_HPP
