/*
 * Copyright (c) 2015-2017, Intel Corporation
 * Copyright (c) 2020-2021, VectorCamp PC
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
#include <cstdio>

#include "ue2common.h"
#include "util/arch.h"
#include "util/unaligned.h"
#include "util/supervector/supervector.hpp"

// 128-bit SSE implementation

template<>
really_inline SuperVector<16>::SuperVector(SuperVector const &other)
{
    u.v128[0] = other.u.v128[0];
}

template<>
really_inline SuperVector<16>::SuperVector(typename base_type::type const v)
{
    u.v128[0] = v;
};

template<>
template<>
really_inline SuperVector<16>::SuperVector<int8_t>(int8_t const other)
{
    u.v128[0] = _mm_set1_epi8(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint8_t>(uint8_t const other)
{
    u.v128[0] = _mm_set1_epi8(static_cast<int8_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int16_t>(int16_t const other)
{
    u.v128[0] = _mm_set1_epi16(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint16_t>(uint16_t const other)
{
    u.v128[0] = _mm_set1_epi16(static_cast<int16_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int32_t>(int32_t const other)
{
    u.v128[0] = _mm_set1_epi32(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint32_t>(uint32_t const other)
{
    u.v128[0] = _mm_set1_epi32(static_cast<int32_t>(other));
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<int64_t>(int64_t const other)
{
    u.v128[0] = _mm_set1_epi64x(other);
}

template<>
template<>
really_inline SuperVector<16>::SuperVector<uint64_t>(uint64_t const other)
{
    u.v128[0] = _mm_set1_epi64x(static_cast<int64_t>(other));
}

// Constants
template<>
really_inline SuperVector<16> SuperVector<16>::Ones(void)
{
    return {_mm_set1_epi8(0xFF)};
}

template<>
really_inline SuperVector<16> SuperVector<16>::Zeroes(void)
{
    return {_mm_set1_epi8(0)};
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
    return {_mm_and_si128(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator|(SuperVector<16> const &b) const
{
    return {_mm_or_si128(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::operator^(SuperVector<16> const &b) const
{
    return {_mm_xor_si128(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::opandnot(SuperVector<16> const &b) const
{
    return {_mm_andnot_si128(u.v128[0], b.u.v128[0])};
}

template <>
really_inline SuperVector<16> SuperVector<16>::eq(SuperVector<16> const &b) const
{
    return {_mm_cmpeq_epi8(u.v128[0], b.u.v128[0])};
}

template <>
really_inline typename SuperVector<16>::movemask_type SuperVector<16>::movemask(void)const
{
    return _mm_movemask_epi8(u.v128[0]);
}

template <>
really_inline typename SuperVector<16>::movemask_type SuperVector<16>::eqmask(SuperVector<16> const b) const
{
    return eq(b).movemask();
}

template <>
really_inline SuperVector<16> SuperVector<16>::rshift128_var(uint8_t const N) const
{
    switch(N) {
    case 1: return {_mm_srli_si128(u.v128[0], 1)}; break;
    case 2: return {_mm_srli_si128(u.v128[0], 2)}; break;
    case 3: return {_mm_srli_si128(u.v128[0], 3)}; break;
    case 4: return {_mm_srli_si128(u.v128[0], 4)}; break;
    case 5: return {_mm_srli_si128(u.v128[0], 5)}; break;
    case 6: return {_mm_srli_si128(u.v128[0], 6)}; break;
    case 7: return {_mm_srli_si128(u.v128[0], 7)}; break;
    case 8: return {_mm_srli_si128(u.v128[0], 8)}; break;
    case 9: return {_mm_srli_si128(u.v128[0], 9)}; break;
    case 10: return {_mm_srli_si128(u.v128[0], 10)}; break;
    case 11: return {_mm_srli_si128(u.v128[0], 11)}; break;
    case 12: return {_mm_srli_si128(u.v128[0], 12)}; break;
    case 13: return {_mm_srli_si128(u.v128[0], 13)}; break;
    case 14: return {_mm_srli_si128(u.v128[0], 14)}; break;
    case 15: return {_mm_srli_si128(u.v128[0], 15)}; break;
    case 16: return Zeroes(); break;
    default: break;
    }
    return *this;
}

#ifdef HS_OPTIMIZE
template <>
really_inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const
{
    return {_mm_srli_si128(u.v128[0], N)};
}
#else
template <>
really_inline SuperVector<16> SuperVector<16>::operator>>(uint8_t const N) const
{
    return rshift128_var(N);
}
#endif

template <>
really_inline SuperVector<16> SuperVector<16>::lshift128_var(uint8_t const N) const
{
    switch(N) {
    case 1: return {_mm_slli_si128(u.v128[0], 1)}; break;
    case 2: return {_mm_slli_si128(u.v128[0], 2)}; break;
    case 3: return {_mm_slli_si128(u.v128[0], 3)}; break;
    case 4: return {_mm_slli_si128(u.v128[0], 4)}; break;
    case 5: return {_mm_slli_si128(u.v128[0], 5)}; break;
    case 6: return {_mm_slli_si128(u.v128[0], 6)}; break;
    case 7: return {_mm_slli_si128(u.v128[0], 7)}; break;
    case 8: return {_mm_slli_si128(u.v128[0], 8)}; break;
    case 9: return {_mm_slli_si128(u.v128[0], 9)}; break;
    case 10: return {_mm_slli_si128(u.v128[0], 10)}; break;
    case 11: return {_mm_slli_si128(u.v128[0], 11)}; break;
    case 12: return {_mm_slli_si128(u.v128[0], 12)}; break;
    case 13: return {_mm_slli_si128(u.v128[0], 13)}; break;
    case 14: return {_mm_slli_si128(u.v128[0], 14)}; break;
    case 15: return {_mm_slli_si128(u.v128[0], 15)}; break;
    case 16: return Zeroes(); break;
    default: break;
    }
    return *this;
}

#ifdef HS_OPTIMIZE
template <>
really_inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const
{
    return {_mm_slli_si128(u.v128[0], N)};
}
#else
template <>
really_inline SuperVector<16> SuperVector<16>::operator<<(uint8_t const N) const
{
    return lshift128_var(N);
}
#endif

template <>
really_inline SuperVector<16> SuperVector<16>::loadu(void const *ptr)
{
    return _mm_loadu_si128((const m128 *)ptr);
}

template <>
really_inline SuperVector<16> SuperVector<16>::load(void const *ptr)
{
    assert(ISALIGNED_N(ptr, alignof(SuperVector::size)));
    ptr = assume_aligned(ptr, SuperVector::size);
    return _mm_load_si128((const m128 *)ptr);
}

template <>
really_inline SuperVector<16> SuperVector<16>::loadu_maskz(void const *ptr, uint8_t const len)
{
    SuperVector<16> mask = Ones().rshift128_var(16 -len);
    mask.print8("mask");
    SuperVector<16> v = _mm_loadu_si128((const m128 *)ptr);
    v.print8("v");
    return mask & v;
}

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset)
{
    return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], offset)};
}
#else
template<>
really_inline SuperVector<16> SuperVector<16>::alignr(SuperVector<16> &other, int8_t offset)
{
    switch(offset) {
    case 0: return other; break;
    case 1: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 1)}; break;
    case 2: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 2)}; break;
    case 3: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 3)}; break;
    case 4: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 4)}; break;
    case 5: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 5)}; break;
    case 6: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 6)}; break;
    case 7: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 7)}; break;
    case 8: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 8)}; break;
    case 9: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 9)}; break;
    case 10: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 10)}; break;
    case 11: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 11)}; break;
    case 12: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 12)}; break;
    case 13: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 13)}; break;
    case 14: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 14)}; break;
    case 15: return {_mm_alignr_epi8(u.v128[0], other.u.v128[0], 15)}; break;
    default: break;
    }
    return *this;
}
#endif

template<>
really_inline SuperVector<16> SuperVector<16>::pshufb(SuperVector<16> b)
{
    return {_mm_shuffle_epi8(u.v128[0], b.u.v128[0])};
}

template<>
really_inline SuperVector<16> SuperVector<16>::pshufb_maskz(SuperVector<16> b, uint8_t const len)
{
    SuperVector<16> mask = Ones().rshift128_var(16 -len);
    return mask & pshufb(b);
}

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<16> SuperVector<16>::lshift64(uint8_t const N)
{
    return {_mm_slli_epi64(u.v128[0], N)};
}
#else
template<>
really_inline SuperVector<16> SuperVector<16>::lshift64(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {_mm_slli_epi64(u.v128[0], 1)}; break;
    case 2: return {_mm_slli_epi64(u.v128[0], 2)}; break;
    case 3: return {_mm_slli_epi64(u.v128[0], 3)}; break;
    case 4: return {_mm_slli_epi64(u.v128[0], 4)}; break;
    case 5: return {_mm_slli_epi64(u.v128[0], 5)}; break;
    case 6: return {_mm_slli_epi64(u.v128[0], 6)}; break;
    case 7: return {_mm_slli_epi64(u.v128[0], 7)}; break;
    case 8: return {_mm_slli_epi64(u.v128[0], 8)}; break;
    case 9: return {_mm_slli_epi64(u.v128[0], 9)}; break;
    case 10: return {_mm_slli_epi64(u.v128[0], 10)}; break;
    case 11: return {_mm_slli_epi64(u.v128[0], 11)}; break;
    case 12: return {_mm_slli_epi64(u.v128[0], 12)}; break;
    case 13: return {_mm_slli_epi64(u.v128[0], 13)}; break;
    case 14: return {_mm_slli_epi64(u.v128[0], 14)}; break;
    case 15: return {_mm_slli_epi64(u.v128[0], 15)}; break;
    case 16: return Zeroes();
    default: break;
    }
    return *this;
}
#endif

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<16> SuperVector<16>::rshift64(uint8_t const N)
{
    return {_mm_srli_epi64(u.v128[0], N)};
}
#else
template<>
really_inline SuperVector<16> SuperVector<16>::rshift64(uint8_t const N)
{
    switch(N) {
    case 0: return {_mm_srli_epi64(u.v128[0], 0)}; break;
    case 1: return {_mm_srli_epi64(u.v128[0], 1)}; break;
    case 2: return {_mm_srli_epi64(u.v128[0], 2)}; break;
    case 3: return {_mm_srli_epi64(u.v128[0], 3)}; break;
    case 4: return {_mm_srli_epi64(u.v128[0], 4)}; break;
    case 5: return {_mm_srli_epi64(u.v128[0], 5)}; break;
    case 6: return {_mm_srli_epi64(u.v128[0], 6)}; break;
    case 7: return {_mm_srli_epi64(u.v128[0], 7)}; break;
    case 8: return {_mm_srli_epi64(u.v128[0], 8)}; break;
    case 9: return {_mm_srli_epi64(u.v128[0], 9)}; break;
    case 10: return {_mm_srli_epi64(u.v128[0], 10)}; break;
    case 11: return {_mm_srli_epi64(u.v128[0], 11)}; break;
    case 12: return {_mm_srli_epi64(u.v128[0], 12)}; break;
    case 13: return {_mm_srli_epi64(u.v128[0], 13)}; break;
    case 14: return {_mm_srli_epi64(u.v128[0], 14)}; break;
    case 15: return {_mm_srli_epi64(u.v128[0], 15)}; break;
        case 16: return Zeroes();
    default: break;
    }
    return *this;
}
#endif

template<>
really_inline SuperVector<16> SuperVector<16>::lshift128(uint8_t const N)
{
    return *this << N;
}

template<>
really_inline SuperVector<16> SuperVector<16>::rshift128(uint8_t const N)
{
    return *this >> N;
}

// 256-bit AVX2 implementation
#if defined(HAVE_AVX2)
template<>
really_inline SuperVector<32>::SuperVector(SuperVector const &other)
{
    u.v256[0] = other.u.v256[0];
}

template<>
really_inline SuperVector<32>::SuperVector(typename base_type::type const v)
{
    u.v256[0] = v;
};

template<>
template<>
really_inline SuperVector<32>::SuperVector(m128 const v)
{
    u.v256[0] = _mm256_broadcastsi128_si256(v);
};

template<>
template<>
really_inline SuperVector<32>::SuperVector<int8_t>(int8_t const other)
{
    u.v256[0] = _mm256_set1_epi8(other);
}

template<>
template<>
really_inline SuperVector<32>::SuperVector<uint8_t>(uint8_t const other)
{
    u.v256[0] = _mm256_set1_epi8(static_cast<int8_t>(other));
}

template<>
template<>
really_inline SuperVector<32>::SuperVector<int16_t>(int16_t const other)
{
    u.v256[0] = _mm256_set1_epi16(other);
}

template<>
template<>
really_inline SuperVector<32>::SuperVector<uint16_t>(uint16_t const other)
{
    u.v256[0] = _mm256_set1_epi16(static_cast<int16_t>(other));
}

template<>
template<>
really_inline SuperVector<32>::SuperVector<int32_t>(int32_t const other)
{
    u.v256[0] = _mm256_set1_epi32(other);
}

template<>
template<>
really_inline SuperVector<32>::SuperVector<uint32_t>(uint32_t const other)
{
    u.v256[0] = _mm256_set1_epi32(static_cast<int32_t>(other));
}

template<>
template<>
really_inline SuperVector<32>::SuperVector<int64_t>(int64_t const other)
{
    u.v256[0] = _mm256_set1_epi64x(other);
}

template<>
template<>
really_inline SuperVector<32>::SuperVector<uint64_t>(uint64_t const other)
{
    u.v256[0] = _mm256_set1_epi64x(static_cast<int64_t>(other));
}

// Constants
template<>
really_inline SuperVector<32> SuperVector<32>::Ones(void)
{
    return {_mm256_set1_epi8(0xFF)};
}

template<>
really_inline SuperVector<32> SuperVector<32>::Zeroes(void)
{
    return {_mm256_set1_epi8(0)};
}

template <>
really_inline void SuperVector<32>::operator=(SuperVector<32> const &other)
{
    u.v256[0] = other.u.v256[0];
}

template <>
really_inline SuperVector<32> SuperVector<32>::operator&(SuperVector<32> const &b) const
{
    return {_mm256_and_si256(u.v256[0], b.u.v256[0])};
}

template <>
really_inline SuperVector<32> SuperVector<32>::operator|(SuperVector<32> const &b) const
{
    return {_mm256_or_si256(u.v256[0], b.u.v256[0])};
}

template <>
really_inline SuperVector<32> SuperVector<32>::operator^(SuperVector<32> const &b) const
{
    return {_mm256_xor_si256(u.v256[0], b.u.v256[0])};
}

template <>
really_inline SuperVector<32> SuperVector<32>::opandnot(SuperVector<32> const &b) const
{
    return {_mm256_andnot_si256(u.v256[0], b.u.v256[0])};
}

template <>
really_inline SuperVector<32> SuperVector<32>::eq(SuperVector<32> const &b) const
{
    return {_mm256_cmpeq_epi8(u.v256[0], b.u.v256[0])};
}

template <>
really_inline typename SuperVector<32>::movemask_type SuperVector<32>::movemask(void)const
{
    return _mm256_movemask_epi8(u.v256[0]);
}

template <>
really_inline typename SuperVector<32>::movemask_type SuperVector<32>::eqmask(SuperVector<32> const b) const
{
    return eq(b).movemask();
}

template <>
really_inline SuperVector<32> SuperVector<32>::rshift128_var(uint8_t const N) const
{
    switch(N) {
    case 1: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 1)}; break;
    case 2: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 2)}; break;
    case 3: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 3)}; break;
    case 4: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 4)}; break;
    case 5: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 5)}; break;
    case 6: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 6)}; break;
    case 7: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 7)}; break;
    case 8: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 8)}; break;
    case 9: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 9)}; break;
    case 10: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 10)}; break;
    case 11: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 11)}; break;
    case 12: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 12)}; break;
    case 13: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 13)}; break;
    case 14: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 14)}; break;
    case 15: return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], 15)}; break;
    case 16: return {_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1))}; break;
    case 17: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 1)}; break;
    case 18: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 2)}; break;
    case 19: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 3)}; break;
    case 20: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 4)}; break;
    case 21: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 5)}; break;
    case 22: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 6)}; break;
    case 23: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 7)}; break;
    case 24: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 8)}; break;
    case 25: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 9)}; break;
    case 26: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 10)}; break;
    case 27: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 11)}; break;
    case 28: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 12)}; break;
    case 29: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 13)}; break;
    case 30: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 14)}; break;
    case 31: return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), 15)}; break;
    case 32: return Zeroes(); break;
    default: break;
    }
    return *this;
}

#ifdef HS_OPTIMIZE
template <>
really_inline SuperVector<32> SuperVector<32>::operator>>(uint8_t const N) const
{
    // As found here: https://stackoverflow.com/questions/25248766/emulating-shifts-on-32-bytes-with-avx
    if (N < 16) {
        return {_mm256_alignr_epi8(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), u.v256[0], N)};
    } else if (N == 16) {
        return {_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1))};
    } else {
        return {_mm256_srli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(2, 0, 0, 1)), N - 16)};
    }
}
#else
template <>
really_inline SuperVector<32> SuperVector<32>::operator>>(uint8_t const N) const
{
    return rshift128_var(N);
}
#endif

template <>
really_inline SuperVector<32> SuperVector<32>::lshift128_var(uint8_t const N) const
{
    switch(N) {
    case 1: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 15)}; break;
    case 2: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 14)}; break;
    case 3: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 13)}; break;
    case 4: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 12)}; break;
    case 5: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 11)}; break;
    case 6: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 10)}; break;
    case 7: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 9)}; break;
    case 8: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 8)}; break;
    case 9: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 7)}; break;
    case 10: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 6)}; break;
    case 11: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 5)}; break;
    case 12: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 4)}; break;
    case 13: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 3)}; break;
    case 14: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 2)}; break;
    case 15: return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 1)}; break;
    case 16: return {_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0))}; break;
    case 17: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 1)}; break;
    case 18: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 2)}; break;
    case 19: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 3)}; break;
    case 20: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 4)}; break;
    case 21: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 5)}; break;
    case 22: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 6)}; break;
    case 23: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 7)}; break;
    case 24: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 8)}; break;
    case 25: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 9)}; break;
    case 26: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 10)}; break;
    case 27: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 11)}; break;
    case 28: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 12)}; break;
    case 29: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 13)}; break;
    case 30: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 14)}; break;
    case 31: return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 15)}; break;
    case 32: return Zeroes(); break;
    default: break;
    }
    return *this;
}

#ifdef HS_OPTIMIZE
template <>
really_inline SuperVector<32> SuperVector<32>::operator<<(uint8_t const N) const
{
    // As found here: https://stackoverflow.com/questions/25248766/emulating-shifts-on-32-bytes-with-avx
    if (N < 16) {
        return {_mm256_alignr_epi8(u.v256[0], _mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), 16 - N)};
    } else if (N == 16) {
        return {_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0))};
    } else {
        return {_mm256_slli_si256(_mm256_permute2x128_si256(u.v256[0], u.v256[0], _MM_SHUFFLE(0, 0, 2, 0)), N - 16)};
    }
}
#else
template <>
really_inline SuperVector<32> SuperVector<32>::operator<<(uint8_t const N) const
{
    return lshift128_var(N);
}
#endif

template <>
really_inline SuperVector<32> SuperVector<32>::loadu(void const *ptr)
{
    return {_mm256_loadu_si256((const m256 *)ptr)};
}

template <>
really_inline SuperVector<32> SuperVector<32>::load(void const *ptr)
{
    assert(ISALIGNED_N(ptr, alignof(SuperVector::size)));
    ptr = assume_aligned(ptr, SuperVector::size);
    return {_mm256_load_si256((const m256 *)ptr)};
}

template <>
really_inline SuperVector<32> SuperVector<32>::loadu_maskz(void const *ptr, uint8_t const len)
{
    SuperVector<32> mask = Ones().rshift128_var(32 -len);
    mask.print8("mask");
    SuperVector<32> v = _mm256_loadu_si256((const m256 *)ptr);
    v.print8("v");
    return mask & v;
}


#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<32> SuperVector<32>::alignr(SuperVector<32> &other, int8_t offset)
{
    return {_mm256_alignr_epi8(u.v256[0], other.u.v256[0], offset)};
}
#else
template<>
really_inline SuperVector<32> SuperVector<32>::alignr(SuperVector<32> &other, int8_t offset)
{
    // As found here: https://stackoverflow.com/questions/8517970/mm-alignr-epi8-palignr-equivalent-in-avx2#8637458
    switch (offset){ 
    case 0 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 0), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 0)); break;
    case 1 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 1), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 1)); break;
    case 2 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 2), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 2)); break;
    case 3 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 3), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 3)); break;
    case 4 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 4), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 4)); break;
    case 5 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 5), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 5)); break;
    case 6 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 6), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 6)); break;
    case 7 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 7), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 7)); break;
    case 8 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 8), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 8)); break;
    case 9 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 9), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 9)); break;
    case 10 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 10), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 10)); break;
    case 11 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 11), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 11)); break;
    case 12 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 12), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 12)); break;
    case 13 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 13), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 13)); break;
    case 14 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 14), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 14)); break;
    case 15 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[0], other.u.v128[1], 15), _mm_alignr_epi8(other.u.v128[1], other.u.v128[0], 15)); break;
    case 16 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 0), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 0)); break;
    case 17 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 1), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 1)); break;
    case 18 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 2), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 2)); break;
    case 19 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 3), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 3)); break;
    case 20 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 4), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 4)); break;
    case 21 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 5), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 5)); break;
    case 22 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 6), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 6)); break;
    case 23 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 7), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 7)); break;
    case 24 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 8), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 8)); break;
    case 25 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 9), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 9)); break;
    case 26 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 10), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 10)); break;
    case 27 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 11), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 11)); break;
    case 28 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 12), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 12)); break;
    case 29 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 13), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 13)); break;
    case 30 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 14), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 14)); break;
    case 31 : return _mm256_set_m128i(_mm_alignr_epi8(u.v128[1], u.v128[0], 15), _mm_alignr_epi8(u.v128[0], other.u.v128[1], 15)); break;  
    default: break;
    }
    return *this;
}
#endif


template<>
really_inline SuperVector<32> SuperVector<32>::pshufb(SuperVector<32> b)
{
    return {_mm256_shuffle_epi8(u.v256[0], b.u.v256[0])};
}

template<>
really_inline SuperVector<32> SuperVector<32>::pshufb_maskz(SuperVector<32> b, uint8_t const len)
{
    SuperVector<32> mask = Ones().rshift128_var(32 -len);
    return mask & pshufb(b);
}

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<32> SuperVector<32>::lshift64(uint8_t const N)
{
    return {_mm256_slli_epi64(u.v256[0], N)};
}
#else
template<>
really_inline SuperVector<32> SuperVector<32>::lshift64(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {_mm256_slli_epi64(u.v256[0], 1)}; break;
    case 2: return {_mm256_slli_epi64(u.v256[0], 2)}; break;
    case 3: return {_mm256_slli_epi64(u.v256[0], 3)}; break;
    case 4: return {_mm256_slli_epi64(u.v256[0], 4)}; break;
    case 5: return {_mm256_slli_epi64(u.v256[0], 5)}; break;
    case 6: return {_mm256_slli_epi64(u.v256[0], 6)}; break;
    case 7: return {_mm256_slli_epi64(u.v256[0], 7)}; break;
    case 8: return {_mm256_slli_epi64(u.v256[0], 8)}; break;
    case 9: return {_mm256_slli_epi64(u.v256[0], 9)}; break;
    case 10: return {_mm256_slli_epi64(u.v256[0], 10)}; break;
    case 11: return {_mm256_slli_epi64(u.v256[0], 11)}; break;
    case 12: return {_mm256_slli_epi64(u.v256[0], 12)}; break;
    case 13: return {_mm256_slli_epi64(u.v256[0], 13)}; break;
    case 14: return {_mm256_slli_epi64(u.v256[0], 14)}; break;
    case 15: return {_mm256_slli_epi64(u.v256[0], 15)}; break;
    case 16: return {_mm256_slli_epi64(u.v256[0], 16)}; break;
    case 17: return {_mm256_slli_epi64(u.v256[0], 17)}; break;
    case 18: return {_mm256_slli_epi64(u.v256[0], 18)}; break;
    case 19: return {_mm256_slli_epi64(u.v256[0], 19)}; break;
    case 20: return {_mm256_slli_epi64(u.v256[0], 20)}; break;
    case 21: return {_mm256_slli_epi64(u.v256[0], 21)}; break;
    case 22: return {_mm256_slli_epi64(u.v256[0], 22)}; break;
    case 23: return {_mm256_slli_epi64(u.v256[0], 23)}; break;
    case 24: return {_mm256_slli_epi64(u.v256[0], 24)}; break;
    case 25: return {_mm256_slli_epi64(u.v256[0], 25)}; break;
    case 26: return {_mm256_slli_epi64(u.v256[0], 26)}; break;
    case 27: return {_mm256_slli_epi64(u.v256[0], 27)}; break;
    case 28: return {_mm256_slli_epi64(u.v256[0], 28)}; break;
    case 29: return {_mm256_slli_epi64(u.v256[0], 29)}; break;
    case 30: return {_mm256_slli_epi64(u.v256[0], 30)}; break;
    case 31: return {_mm256_slli_epi64(u.v256[0], 31)}; break;
        case 32: return Zeroes();
    default: break;
    }
    return *this;
}
#endif

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<32> SuperVector<32>::rshift64(uint8_t const N)
{
    return {_mm256_srli_epi64(u.v256[0], N)};
}
#else
template<>
really_inline SuperVector<32> SuperVector<32>::rshift64(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {_mm256_srli_epi64(u.v256[0], 1)}; break;
    case 2: return {_mm256_srli_epi64(u.v256[0], 2)}; break;
    case 3: return {_mm256_srli_epi64(u.v256[0], 3)}; break;
    case 4: return {_mm256_srli_epi64(u.v256[0], 4)}; break;
    case 5: return {_mm256_srli_epi64(u.v256[0], 5)}; break;
    case 6: return {_mm256_srli_epi64(u.v256[0], 6)}; break;
    case 7: return {_mm256_srli_epi64(u.v256[0], 7)}; break;
    case 8: return {_mm256_srli_epi64(u.v256[0], 8)}; break;
    case 9: return {_mm256_srli_epi64(u.v256[0], 9)}; break;
    case 10: return {_mm256_srli_epi64(u.v256[0], 10)}; break;
    case 11: return {_mm256_srli_epi64(u.v256[0], 11)}; break;
    case 12: return {_mm256_srli_epi64(u.v256[0], 12)}; break;
    case 13: return {_mm256_srli_epi64(u.v256[0], 13)}; break;
    case 14: return {_mm256_srli_epi64(u.v256[0], 14)}; break;
    case 15: return {_mm256_srli_epi64(u.v256[0], 15)}; break;
    case 16: return {_mm256_srli_epi64(u.v256[0], 16)}; break;
    case 17: return {_mm256_srli_epi64(u.v256[0], 17)}; break;
    case 18: return {_mm256_srli_epi64(u.v256[0], 18)}; break;
    case 19: return {_mm256_srli_epi64(u.v256[0], 19)}; break;
    case 20: return {_mm256_srli_epi64(u.v256[0], 20)}; break;
    case 21: return {_mm256_srli_epi64(u.v256[0], 21)}; break;
    case 22: return {_mm256_srli_epi64(u.v256[0], 22)}; break;
    case 23: return {_mm256_srli_epi64(u.v256[0], 23)}; break;
    case 24: return {_mm256_srli_epi64(u.v256[0], 24)}; break;
    case 25: return {_mm256_srli_epi64(u.v256[0], 25)}; break;
    case 26: return {_mm256_srli_epi64(u.v256[0], 26)}; break;
    case 27: return {_mm256_srli_epi64(u.v256[0], 27)}; break;
    case 28: return {_mm256_srli_epi64(u.v256[0], 28)}; break;
    case 29: return {_mm256_srli_epi64(u.v256[0], 29)}; break;
    case 30: return {_mm256_srli_epi64(u.v256[0], 30)}; break;
    case 31: return {_mm256_srli_epi64(u.v256[0], 31)}; break;
        case 32: return Zeroes();
    default: break;
    }
    return *this;
}
#endif

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<32> SuperVector<32>::lshift128(uint8_t const N)
{
    return {_mm256_slli_si256(u.v256[0], N)};
}
#else
template<>
really_inline SuperVector<32> SuperVector<32>::lshift128(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {_mm256_slli_si256(u.v256[0], 1)}; break;
    case 2: return {_mm256_slli_si256(u.v256[0], 2)}; break;
    case 3: return {_mm256_slli_si256(u.v256[0], 3)}; break;
    case 4: return {_mm256_slli_si256(u.v256[0], 4)}; break;
    case 5: return {_mm256_slli_si256(u.v256[0], 5)}; break;
    case 6: return {_mm256_slli_si256(u.v256[0], 6)}; break;
    case 7: return {_mm256_slli_si256(u.v256[0], 7)}; break;
    case 8: return {_mm256_slli_si256(u.v256[0], 8)}; break;
    case 9: return {_mm256_slli_si256(u.v256[0], 9)}; break;
    case 10: return {_mm256_slli_si256(u.v256[0], 10)}; break;
    case 11: return {_mm256_slli_si256(u.v256[0], 11)}; break;
    case 12: return {_mm256_slli_si256(u.v256[0], 12)}; break;
    case 13: return {_mm256_slli_si256(u.v256[0], 13)}; break;
    case 14: return {_mm256_slli_si256(u.v256[0], 14)}; break;
    case 15: return {_mm256_slli_si256(u.v256[0], 15)}; break;
    case 16: return {_mm256_slli_si256(u.v256[0], 16)}; break;
    case 17: return {_mm256_slli_si256(u.v256[0], 17)}; break;
    case 18: return {_mm256_slli_si256(u.v256[0], 18)}; break;
    case 19: return {_mm256_slli_si256(u.v256[0], 19)}; break;
    case 20: return {_mm256_slli_si256(u.v256[0], 20)}; break;
    case 21: return {_mm256_slli_si256(u.v256[0], 21)}; break;
    case 22: return {_mm256_slli_si256(u.v256[0], 22)}; break;
    case 23: return {_mm256_slli_si256(u.v256[0], 23)}; break;
    case 24: return {_mm256_slli_si256(u.v256[0], 24)}; break;
    case 25: return {_mm256_slli_si256(u.v256[0], 25)}; break;
    case 26: return {_mm256_slli_si256(u.v256[0], 26)}; break;
    case 27: return {_mm256_slli_si256(u.v256[0], 27)}; break;
    case 28: return {_mm256_slli_si256(u.v256[0], 28)}; break;
    case 29: return {_mm256_slli_si256(u.v256[0], 29)}; break;
    case 30: return {_mm256_slli_si256(u.v256[0], 30)}; break;
    case 31: return {_mm256_slli_si256(u.v256[0], 31)}; break;
    default: break;
    }
    return *this;
}
#endif

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<32> SuperVector<32>::rshift128(uint8_t const N)
{
    return {_mm256_srli_si256(u.v256[0], N)};
}
#else
template<>
really_inline SuperVector<32> SuperVector<32>::rshift128(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {_mm256_srli_si256(u.v256[0], 1)}; break;
    case 2: return {_mm256_srli_si256(u.v256[0], 2)}; break;
    case 3: return {_mm256_srli_si256(u.v256[0], 3)}; break;
    case 4: return {_mm256_srli_si256(u.v256[0], 4)}; break;
    case 5: return {_mm256_srli_si256(u.v256[0], 5)}; break;
    case 6: return {_mm256_srli_si256(u.v256[0], 6)}; break;
    case 7: return {_mm256_srli_si256(u.v256[0], 7)}; break;
    case 8: return {_mm256_srli_si256(u.v256[0], 8)}; break;
    case 9: return {_mm256_srli_si256(u.v256[0], 9)}; break;
    case 10: return {_mm256_srli_si256(u.v256[0], 10)}; break;
    case 11: return {_mm256_srli_si256(u.v256[0], 11)}; break;
    case 12: return {_mm256_srli_si256(u.v256[0], 12)}; break;
    case 13: return {_mm256_srli_si256(u.v256[0], 13)}; break;
    case 14: return {_mm256_srli_si256(u.v256[0], 14)}; break;
    case 15: return {_mm256_srli_si256(u.v256[0], 15)}; break;
    case 16: return {_mm256_srli_si256(u.v256[0], 16)}; break;
    case 17: return {_mm256_srli_si256(u.v256[0], 17)}; break;
    case 18: return {_mm256_srli_si256(u.v256[0], 18)}; break;
    case 19: return {_mm256_srli_si256(u.v256[0], 19)}; break;
    case 20: return {_mm256_srli_si256(u.v256[0], 20)}; break;
    case 21: return {_mm256_srli_si256(u.v256[0], 21)}; break;
    case 22: return {_mm256_srli_si256(u.v256[0], 22)}; break;
    case 23: return {_mm256_srli_si256(u.v256[0], 23)}; break;
    case 24: return {_mm256_srli_si256(u.v256[0], 24)}; break;
    case 25: return {_mm256_srli_si256(u.v256[0], 25)}; break;
    case 26: return {_mm256_srli_si256(u.v256[0], 26)}; break;
    case 27: return {_mm256_srli_si256(u.v256[0], 27)}; break;
    case 28: return {_mm256_srli_si256(u.v256[0], 28)}; break;
    case 29: return {_mm256_srli_si256(u.v256[0], 29)}; break;
    case 30: return {_mm256_srli_si256(u.v256[0], 30)}; break;
    case 31: return {_mm256_srli_si256(u.v256[0], 31)}; break;
    default: break;
    }
    return *this;
}
#endif

#endif // HAVE_AVX2


// 512-bit AVX512 implementation
#if defined(HAVE_AVX512)
template<>
really_inline SuperVector<64>::SuperVector(SuperVector const &o)
{
    u.v512[0] = o.u.v512[0];
}

template<>
really_inline SuperVector<64>::SuperVector(typename base_type::type const v)
{
    u.v512[0] = v;
};

template<>
template<>
really_inline SuperVector<64>::SuperVector(m256 const v)
{
    u.v512[0] = _mm512_broadcast_i64x4(v);
};

template<>
really_inline SuperVector<64>::SuperVector(m256 const lo, m256 const hi)
{
    u.v256[0] = lo;
    u.v256[1] = hi;
};

template<>
really_inline SuperVector<64>::SuperVector(SuperVector<32> const lo, SuperVector<32> const hi)
{
    u.v256[0] = lo.u.v256[0];
    u.v256[1] = hi.u.v256[0];
};

template<>
template<>
really_inline SuperVector<64>::SuperVector(m128 const v)
{
    u.v512[0] = _mm512_broadcast_i32x4(v);
};

template<>
template<>
really_inline SuperVector<64>::SuperVector<int8_t>(int8_t const o)
{
    u.v512[0] = _mm512_set1_epi8(o);
}

template<>
template<>
really_inline SuperVector<64>::SuperVector<uint8_t>(uint8_t const o)
{
    u.v512[0] = _mm512_set1_epi8(static_cast<int8_t>(o));
}

template<>
template<>
really_inline SuperVector<64>::SuperVector<int16_t>(int16_t const o)
{
    u.v512[0] = _mm512_set1_epi16(o);
}

template<>
template<>
really_inline SuperVector<64>::SuperVector<uint16_t>(uint16_t const o)
{
    u.v512[0] = _mm512_set1_epi16(static_cast<int16_t>(o));
}

template<>
template<>
really_inline SuperVector<64>::SuperVector<int32_t>(int32_t const o)
{
    u.v512[0] = _mm512_set1_epi32(o);
}

template<>
template<>
really_inline SuperVector<64>::SuperVector<uint32_t>(uint32_t const o)
{
    u.v512[0] = _mm512_set1_epi32(static_cast<int32_t>(o));
}

template<>
template<>
really_inline SuperVector<64>::SuperVector<int64_t>(int64_t const o)
{
    u.v512[0] = _mm512_set1_epi64(o);
}

template<>
template<>
really_inline SuperVector<64>::SuperVector<uint64_t>(uint64_t const o)
{
    u.v512[0] = _mm512_set1_epi64(static_cast<int64_t>(o));
}

// Constants
template<>
really_inline SuperVector<64> SuperVector<64>::Ones(void)
{
    return {_mm512_set1_epi8(0xFF)};
}

template<>
really_inline SuperVector<64> SuperVector<64>::Zeroes(void)
{
    return {_mm512_set1_epi8(0)};
}


// Methods
template <>
really_inline void SuperVector<64>::operator=(SuperVector<64> const &o)
{
    u.v512[0] = o.u.v512[0];
}

template <>
really_inline SuperVector<64> SuperVector<64>::operator&(SuperVector<64> const &b) const
{
    return {_mm512_and_si512(u.v512[0], b.u.v512[0])};
}

template <>
really_inline SuperVector<64> SuperVector<64>::operator|(SuperVector<64> const &b) const
{
    return {_mm512_or_si512(u.v512[0], b.u.v512[0])};
}

template <>
really_inline SuperVector<64> SuperVector<64>::operator^(SuperVector<64> const &b) const
{
    return {_mm512_xor_si512(u.v512[0], b.u.v512[0])};
}

template <>
really_inline SuperVector<64> SuperVector<64>::opandnot(SuperVector<64> const &b) const
{
    return {_mm512_andnot_si512(u.v512[0], b.u.v512[0])};
}

template <>
really_inline SuperVector<64> SuperVector<64>::eq(SuperVector<64> const &b) const
{
    m512_t sp = SuperVector<64>::Zeroes();
    sp.u.v256[0] = _mm256_cmpeq_epi8(u.v256[0], b.u.v256[0]);
    sp.u.v256[1] = _mm256_cmpeq_epi8(u.v256[1], b.u.v256[1]);
    return {sp.u.v512[0]};
}

template <>
really_inline typename SuperVector<64>::movemask_type SuperVector<64>::movemask(void)const
{   
    m512_t msb = SuperVector<64>::dup_u8(0x80);
    m512_t mask = msb & *this;
    return _mm512_cmpeq_epi8_mask(mask.u.v512[0],msb.u.v512[0]);
}

template <>
really_inline typename SuperVector<64>::movemask_type SuperVector<64>::eqmask(SuperVector<64> const b) const
{
    return _mm512_cmpeq_epi8_mask(u.v512[0], b.u.v512[0]);
}

template <>
really_inline SuperVector<64> SuperVector<64>::operator>>(uint8_t const N) const
{
    if (N == 0) {
        return *this;
    } else if (N < 32) {
        SuperVector<32> lo256 = u.v256[0];
        SuperVector<32> hi256 = u.v256[1];
        SuperVector<32> carry = hi256 << (32 - N);
        hi256 = hi256 >> N;
        lo256 = (lo256 >> N) | carry;
        return SuperVector(lo256, hi256);
    } else if (N == 32) {
        SuperVector<32> hi256 = u.v256[1];
        return SuperVector(hi256, SuperVector<32>::Zeroes());
    } else if (N < 64) {
        SuperVector<32> hi256 = u.v256[1];
        return SuperVector(hi256 >> (N - 32), SuperVector<32>::Zeroes());
    } else {
        return Zeroes();
    }
}

template <>
really_inline SuperVector<64> SuperVector<64>::operator<<(uint8_t const N) const
{
    if (N == 0) {
        return *this;
    } else if (N < 32) {
        SuperVector<32> lo256 = u.v256[0];
        SuperVector<32> hi256 = u.v256[1];
        SuperVector<32> carry = lo256 >> (32 - N);
        hi256 = (hi256 << N) | carry;
        lo256 = lo256 << N;
        return SuperVector(lo256, hi256);
    } else if (N == 32) {
        SuperVector<32> lo256 = u.v256[0];
        return SuperVector(SuperVector<32>::Zeroes(), lo256);
    } else if (N < 64) {
        SuperVector<32> lo256 = u.v256[0];
        return SuperVector(SuperVector<32>::Zeroes(), lo256 << (N - 32));
    } else {
        return Zeroes();
    }
}

template <>
really_inline SuperVector<64> SuperVector<64>::loadu(void const *ptr)
{
    return {_mm512_loadu_si512((const m512 *)ptr)};
}

template <>
really_inline SuperVector<64> SuperVector<64>::load(void const *ptr)
{
    assert(ISALIGNED_N(ptr, alignof(SuperVector::size)));
    ptr = assume_aligned(ptr, SuperVector::size);
    return {_mm512_load_si512((const m512 *)ptr)};
}

template <>
really_inline SuperVector<64> SuperVector<64>::loadu_maskz(void const *ptr, uint8_t const len)
{
    u64a mask = (~0ULL) >> (64 - len);
    DEBUG_PRINTF("mask = %016llx\n", mask);
    SuperVector<64> v = _mm512_mask_loadu_epi8(Zeroes().u.v512[0], mask, (const m512 *)ptr);
    v.print8("v");
    return v;
}

template<>
really_inline SuperVector<64> SuperVector<64>::pshufb(SuperVector<64> b)
{
    return {_mm512_shuffle_epi8(u.v512[0], b.u.v512[0])};
}

template<>
really_inline SuperVector<64> SuperVector<64>::pshufb_maskz(SuperVector<64> b, uint8_t const len)
{
    u64a mask = (~0ULL) >> (64 - len);
    DEBUG_PRINTF("mask = %016llx\n", mask);
    return {_mm512_maskz_shuffle_epi8(mask, u.v512[0], b.u.v512[0])};
}

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<64> SuperVector<64>::alignr(SuperVector<64> &l, int8_t offset)
{
    return {_mm512_alignr_epi8(u.v512[0], l.u.v512[0], offset)};
}
#else
template<>
really_inline SuperVector<64> SuperVector<64>::alignr(SuperVector<64> &l, int8_t offset)
{
    if(offset == 0){
        return *this;
    } else if (offset < 32){
        SuperVector<32> lo256 = u.v256[0];
        SuperVector<32> hi256 = u.v256[1];
        SuperVector<32> o_lo256 = l.u.v256[0];
        SuperVector<32> carry1 = hi256.alignr(lo256,offset);
        SuperVector<32> carry2 = o_lo256.alignr(hi256,offset);
        return SuperVector(carry1, carry2);
    } else if (offset <= 64){
        SuperVector<32> hi256 = u.v256[1];
        SuperVector<32> o_lo256 = l.u.v256[0];
        SuperVector<32> o_hi256 = l.u.v256[1];
        SuperVector<32> carry1 = o_lo256.alignr(hi256, offset - 32);
        SuperVector<32> carry2 = o_hi256.alignr(o_lo256,offset -32);
        return SuperVector(carry1, carry2);
    } else {
        return *this;
    }
}
#endif


#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<64> SuperVector<64>::lshift64(uint8_t const N)
{
    return {_mm512_slli_epi64(u.v512[0], N)};
}
#else
template<>
really_inline SuperVector<64> SuperVector<64>::lshift64(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {_mm512_slli_epi64(u.v512[0], 1)}; break;
    case 2: return {_mm512_slli_epi64(u.v512[0], 2)}; break;
    case 3: return {_mm512_slli_epi64(u.v512[0], 3)}; break;
    case 4: return {_mm512_slli_epi64(u.v512[0], 4)}; break;
    case 5: return {_mm512_slli_epi64(u.v512[0], 5)}; break;
    case 6: return {_mm512_slli_epi64(u.v512[0], 6)}; break;
    case 7: return {_mm512_slli_epi64(u.v512[0], 7)}; break;
    case 8: return {_mm512_slli_epi64(u.v512[0], 8)}; break;
    case 9: return {_mm512_slli_epi64(u.v512[0], 9)}; break;
    case 10: return {_mm512_slli_epi64(u.v512[0], 10)}; break;
    case 11: return {_mm512_slli_epi64(u.v512[0], 11)}; break;
    case 12: return {_mm512_slli_epi64(u.v512[0], 12)}; break;
    case 13: return {_mm512_slli_epi64(u.v512[0], 13)}; break;
    case 14: return {_mm512_slli_epi64(u.v512[0], 14)}; break;
    case 15: return {_mm512_slli_epi64(u.v512[0], 15)}; break;
    case 16: return {_mm512_slli_epi64(u.v512[0], 16)}; break;
    case 17: return {_mm512_slli_epi64(u.v512[0], 17)}; break;
    case 18: return {_mm512_slli_epi64(u.v512[0], 18)}; break;
    case 19: return {_mm512_slli_epi64(u.v512[0], 19)}; break;
    case 20: return {_mm512_slli_epi64(u.v512[0], 20)}; break;
    case 21: return {_mm512_slli_epi64(u.v512[0], 21)}; break;
    case 22: return {_mm512_slli_epi64(u.v512[0], 22)}; break;
    case 23: return {_mm512_slli_epi64(u.v512[0], 23)}; break;
    case 24: return {_mm512_slli_epi64(u.v512[0], 24)}; break;
    case 25: return {_mm512_slli_epi64(u.v512[0], 25)}; break;
    case 26: return {_mm512_slli_epi64(u.v512[0], 26)}; break;
    case 27: return {_mm512_slli_epi64(u.v512[0], 27)}; break;
    case 28: return {_mm512_slli_epi64(u.v512[0], 28)}; break;
    case 29: return {_mm512_slli_epi64(u.v512[0], 29)}; break;
    case 30: return {_mm512_slli_epi64(u.v512[0], 30)}; break;
    case 31: return {_mm512_slli_epi64(u.v512[0], 31)}; break;
    case 32: return {_mm512_slli_epi64(u.v512[0], 32)}; break;
    case 33: return {_mm512_slli_epi64(u.v512[0], 33)}; break;
    case 34: return {_mm512_slli_epi64(u.v512[0], 34)}; break;
    case 35: return {_mm512_slli_epi64(u.v512[0], 35)}; break;
    case 36: return {_mm512_slli_epi64(u.v512[0], 36)}; break;
    case 37: return {_mm512_slli_epi64(u.v512[0], 37)}; break;
    case 38: return {_mm512_slli_epi64(u.v512[0], 38)}; break;
    case 39: return {_mm512_slli_epi64(u.v512[0], 39)}; break;
    case 40: return {_mm512_slli_epi64(u.v512[0], 40)}; break;
    case 41: return {_mm512_slli_epi64(u.v512[0], 41)}; break;
    case 42: return {_mm512_slli_epi64(u.v512[0], 42)}; break;
    case 43: return {_mm512_slli_epi64(u.v512[0], 43)}; break;
    case 44: return {_mm512_slli_epi64(u.v512[0], 44)}; break;
    case 45: return {_mm512_slli_epi64(u.v512[0], 45)}; break;
    case 46: return {_mm512_slli_epi64(u.v512[0], 46)}; break;
    case 47: return {_mm512_slli_epi64(u.v512[0], 47)}; break;
    case 48: return {_mm512_slli_epi64(u.v512[0], 48)}; break;
    case 49: return {_mm512_slli_epi64(u.v512[0], 49)}; break;
    case 50: return {_mm512_slli_epi64(u.v512[0], 50)}; break;
    case 51: return {_mm512_slli_epi64(u.v512[0], 51)}; break;
    case 52: return {_mm512_slli_epi64(u.v512[0], 52)}; break;
    case 53: return {_mm512_slli_epi64(u.v512[0], 53)}; break;
    case 54: return {_mm512_slli_epi64(u.v512[0], 54)}; break;
    case 55: return {_mm512_slli_epi64(u.v512[0], 55)}; break;
    case 56: return {_mm512_slli_epi64(u.v512[0], 56)}; break;
    case 57: return {_mm512_slli_epi64(u.v512[0], 57)}; break;
    case 58: return {_mm512_slli_epi64(u.v512[0], 58)}; break;
    case 59: return {_mm512_slli_epi64(u.v512[0], 59)}; break;
    case 60: return {_mm512_slli_epi64(u.v512[0], 60)}; break;
    case 61: return {_mm512_slli_epi64(u.v512[0], 61)}; break;
    case 62: return {_mm512_slli_epi64(u.v512[0], 62)}; break;
    case 63: return {_mm512_slli_epi64(u.v512[0], 63)}; break;
    case 64: return Zeroes();
    default: break;
    }
    return *this;
}
#endif

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<64> SuperVector<64>::rshift64(uint8_t const N)
{
    return {_mm512_srli_epi64(u.v512[0], N)};
}
#else
template<>
really_inline SuperVector<64> SuperVector<64>::rshift64(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {_mm512_srli_epi64(u.v512[0], 1)}; break;
    case 2: return {_mm512_srli_epi64(u.v512[0], 2)}; break;
    case 3: return {_mm512_srli_epi64(u.v512[0], 3)}; break;
    case 4: return {_mm512_srli_epi64(u.v512[0], 4)}; break;
    case 5: return {_mm512_srli_epi64(u.v512[0], 5)}; break;
    case 6: return {_mm512_srli_epi64(u.v512[0], 6)}; break;
    case 7: return {_mm512_srli_epi64(u.v512[0], 7)}; break;
    case 8: return {_mm512_srli_epi64(u.v512[0], 8)}; break;
    case 9: return {_mm512_srli_epi64(u.v512[0], 9)}; break;
    case 10: return {_mm512_srli_epi64(u.v512[0], 10)}; break;
    case 11: return {_mm512_srli_epi64(u.v512[0], 11)}; break;
    case 12: return {_mm512_srli_epi64(u.v512[0], 12)}; break;
    case 13: return {_mm512_srli_epi64(u.v512[0], 13)}; break;
    case 14: return {_mm512_srli_epi64(u.v512[0], 14)}; break;
    case 15: return {_mm512_srli_epi64(u.v512[0], 15)}; break;
    case 16: return {_mm512_srli_epi64(u.v512[0], 16)}; break;
    case 17: return {_mm512_srli_epi64(u.v512[0], 17)}; break;
    case 18: return {_mm512_srli_epi64(u.v512[0], 18)}; break;
    case 19: return {_mm512_srli_epi64(u.v512[0], 19)}; break;
    case 20: return {_mm512_srli_epi64(u.v512[0], 20)}; break;
    case 21: return {_mm512_srli_epi64(u.v512[0], 21)}; break;
    case 22: return {_mm512_srli_epi64(u.v512[0], 22)}; break;
    case 23: return {_mm512_srli_epi64(u.v512[0], 23)}; break;
    case 24: return {_mm512_srli_epi64(u.v512[0], 24)}; break;
    case 25: return {_mm512_srli_epi64(u.v512[0], 25)}; break;
    case 26: return {_mm512_srli_epi64(u.v512[0], 26)}; break;
    case 27: return {_mm512_srli_epi64(u.v512[0], 27)}; break;
    case 28: return {_mm512_srli_epi64(u.v512[0], 28)}; break;
    case 29: return {_mm512_srli_epi64(u.v512[0], 29)}; break;
    case 30: return {_mm512_srli_epi64(u.v512[0], 30)}; break;
    case 31: return {_mm512_srli_epi64(u.v512[0], 31)}; break;
    case 32: return {_mm512_srli_epi64(u.v512[0], 32)}; break;
    case 33: return {_mm512_srli_epi64(u.v512[0], 33)}; break;
    case 34: return {_mm512_srli_epi64(u.v512[0], 34)}; break;
    case 35: return {_mm512_srli_epi64(u.v512[0], 35)}; break;
    case 36: return {_mm512_srli_epi64(u.v512[0], 36)}; break;
    case 37: return {_mm512_srli_epi64(u.v512[0], 37)}; break;
    case 38: return {_mm512_srli_epi64(u.v512[0], 38)}; break;
    case 39: return {_mm512_srli_epi64(u.v512[0], 39)}; break;
    case 40: return {_mm512_srli_epi64(u.v512[0], 40)}; break;
    case 41: return {_mm512_srli_epi64(u.v512[0], 41)}; break;
    case 42: return {_mm512_srli_epi64(u.v512[0], 42)}; break;
    case 43: return {_mm512_srli_epi64(u.v512[0], 43)}; break;
    case 44: return {_mm512_srli_epi64(u.v512[0], 44)}; break;
    case 45: return {_mm512_srli_epi64(u.v512[0], 45)}; break;
    case 46: return {_mm512_srli_epi64(u.v512[0], 46)}; break;
    case 47: return {_mm512_srli_epi64(u.v512[0], 47)}; break;
    case 48: return {_mm512_srli_epi64(u.v512[0], 48)}; break;
    case 49: return {_mm512_srli_epi64(u.v512[0], 49)}; break;
    case 50: return {_mm512_srli_epi64(u.v512[0], 50)}; break;
    case 51: return {_mm512_srli_epi64(u.v512[0], 51)}; break;
    case 52: return {_mm512_srli_epi64(u.v512[0], 52)}; break;
    case 53: return {_mm512_srli_epi64(u.v512[0], 53)}; break;
    case 54: return {_mm512_srli_epi64(u.v512[0], 54)}; break;
    case 55: return {_mm512_srli_epi64(u.v512[0], 55)}; break;
    case 56: return {_mm512_srli_epi64(u.v512[0], 56)}; break;
    case 57: return {_mm512_srli_epi64(u.v512[0], 57)}; break;
    case 58: return {_mm512_srli_epi64(u.v512[0], 58)}; break;
    case 59: return {_mm512_srli_epi64(u.v512[0], 59)}; break;
    case 60: return {_mm512_srli_epi64(u.v512[0], 60)}; break;
    case 61: return {_mm512_srli_epi64(u.v512[0], 61)}; break;
    case 62: return {_mm512_srli_epi64(u.v512[0], 62)}; break;
    case 63: return {_mm512_srli_epi64(u.v512[0], 63)}; break;
    case 64: return Zeroes();
    default: break;
    }
    return *this;
}
#endif

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<64> SuperVector<64>::lshift128(uint8_t const N)
{
    return {_mm512_bslli_epi128(u.v512[0], N)};
}
#else
template<>
really_inline SuperVector<64> SuperVector<64>::lshift128(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {_mm512_bslli_epi128(u.v512[0], 1)}; break;
    case 2: return {_mm512_bslli_epi128(u.v512[0], 2)}; break;
    case 3: return {_mm512_bslli_epi128(u.v512[0], 3)}; break;
    case 4: return {_mm512_bslli_epi128(u.v512[0], 4)}; break;
    case 5: return {_mm512_bslli_epi128(u.v512[0], 5)}; break;
    case 6: return {_mm512_bslli_epi128(u.v512[0], 6)}; break;
    case 7: return {_mm512_bslli_epi128(u.v512[0], 7)}; break;
    case 8: return {_mm512_bslli_epi128(u.v512[0], 8)}; break;
    case 9: return {_mm512_bslli_epi128(u.v512[0], 9)}; break;
    case 10: return {_mm512_bslli_epi128(u.v512[0], 10)}; break;
    case 11: return {_mm512_bslli_epi128(u.v512[0], 11)}; break;
    case 12: return {_mm512_bslli_epi128(u.v512[0], 12)}; break;
    case 13: return {_mm512_bslli_epi128(u.v512[0], 13)}; break;
    case 14: return {_mm512_bslli_epi128(u.v512[0], 14)}; break;
    case 15: return {_mm512_bslli_epi128(u.v512[0], 15)}; break;
    case 16: return {_mm512_bslli_epi128(u.v512[0], 16)}; break;
    case 17: return {_mm512_bslli_epi128(u.v512[0], 17)}; break;
    case 18: return {_mm512_bslli_epi128(u.v512[0], 18)}; break;
    case 19: return {_mm512_bslli_epi128(u.v512[0], 19)}; break;
    case 20: return {_mm512_bslli_epi128(u.v512[0], 20)}; break;
    case 21: return {_mm512_bslli_epi128(u.v512[0], 21)}; break;
    case 22: return {_mm512_bslli_epi128(u.v512[0], 22)}; break;
    case 23: return {_mm512_bslli_epi128(u.v512[0], 23)}; break;
    case 24: return {_mm512_bslli_epi128(u.v512[0], 24)}; break;
    case 25: return {_mm512_bslli_epi128(u.v512[0], 25)}; break;
    case 26: return {_mm512_bslli_epi128(u.v512[0], 26)}; break;
    case 27: return {_mm512_bslli_epi128(u.v512[0], 27)}; break;
    case 28: return {_mm512_bslli_epi128(u.v512[0], 28)}; break;
    case 29: return {_mm512_bslli_epi128(u.v512[0], 29)}; break;
    case 30: return {_mm512_bslli_epi128(u.v512[0], 30)}; break;
    case 31: return {_mm512_bslli_epi128(u.v512[0], 31)}; break;
    case 32: return {_mm512_bslli_epi128(u.v512[0], 32)}; break;
    case 33: return {_mm512_bslli_epi128(u.v512[0], 33)}; break;
    case 34: return {_mm512_bslli_epi128(u.v512[0], 34)}; break;
    case 35: return {_mm512_bslli_epi128(u.v512[0], 35)}; break;
    case 36: return {_mm512_bslli_epi128(u.v512[0], 36)}; break;
    case 37: return {_mm512_bslli_epi128(u.v512[0], 37)}; break;
    case 38: return {_mm512_bslli_epi128(u.v512[0], 38)}; break;
    case 39: return {_mm512_bslli_epi128(u.v512[0], 39)}; break;
    case 40: return {_mm512_bslli_epi128(u.v512[0], 40)}; break;
    case 41: return {_mm512_bslli_epi128(u.v512[0], 41)}; break;
    case 42: return {_mm512_bslli_epi128(u.v512[0], 42)}; break;
    case 43: return {_mm512_bslli_epi128(u.v512[0], 43)}; break;
    case 44: return {_mm512_bslli_epi128(u.v512[0], 44)}; break;
    case 45: return {_mm512_bslli_epi128(u.v512[0], 45)}; break;
    case 46: return {_mm512_bslli_epi128(u.v512[0], 46)}; break;
    case 47: return {_mm512_bslli_epi128(u.v512[0], 47)}; break;
    case 48: return {_mm512_bslli_epi128(u.v512[0], 48)}; break;
    case 49: return {_mm512_bslli_epi128(u.v512[0], 49)}; break;
    case 50: return {_mm512_bslli_epi128(u.v512[0], 50)}; break;
    case 51: return {_mm512_bslli_epi128(u.v512[0], 51)}; break;
    case 52: return {_mm512_bslli_epi128(u.v512[0], 52)}; break;
    case 53: return {_mm512_bslli_epi128(u.v512[0], 53)}; break;
    case 54: return {_mm512_bslli_epi128(u.v512[0], 54)}; break;
    case 55: return {_mm512_bslli_epi128(u.v512[0], 55)}; break;
    case 56: return {_mm512_bslli_epi128(u.v512[0], 56)}; break;
    case 57: return {_mm512_bslli_epi128(u.v512[0], 57)}; break;
    case 58: return {_mm512_bslli_epi128(u.v512[0], 58)}; break;
    case 59: return {_mm512_bslli_epi128(u.v512[0], 59)}; break;
    case 60: return {_mm512_bslli_epi128(u.v512[0], 60)}; break;
    case 61: return {_mm512_bslli_epi128(u.v512[0], 61)}; break;
    case 62: return {_mm512_bslli_epi128(u.v512[0], 62)}; break;
    case 63: return {_mm512_bslli_epi128(u.v512[0], 63)}; break;
    case 64: return Zeroes();
    default: break;
    }
    return *this;
}
#endif

#ifdef HS_OPTIMIZE
template<>
really_inline SuperVector<64> SuperVector<64>::rshift128(uint8_t const N)
{
    return {_mm512_bsrli_epi128(u.v512[0], N)};
}
#else
template<>
really_inline SuperVector<64> SuperVector<64>::rshift128(uint8_t const N)
{
    switch(N) {
    case 0: return *this; break;
    case 1: return {_mm512_bsrli_epi128(u.v512[0], 1)}; break;
    case 2: return {_mm512_bsrli_epi128(u.v512[0], 2)}; break;
    case 3: return {_mm512_bsrli_epi128(u.v512[0], 3)}; break;
    case 4: return {_mm512_bsrli_epi128(u.v512[0], 4)}; break;
    case 5: return {_mm512_bsrli_epi128(u.v512[0], 5)}; break;
    case 6: return {_mm512_bsrli_epi128(u.v512[0], 6)}; break;
    case 7: return {_mm512_bsrli_epi128(u.v512[0], 7)}; break;
    case 8: return {_mm512_bsrli_epi128(u.v512[0], 8)}; break;
    case 9: return {_mm512_bsrli_epi128(u.v512[0], 9)}; break;
    case 10: return {_mm512_bsrli_epi128(u.v512[0], 10)}; break;
    case 11: return {_mm512_bsrli_epi128(u.v512[0], 11)}; break;
    case 12: return {_mm512_bsrli_epi128(u.v512[0], 12)}; break;
    case 13: return {_mm512_bsrli_epi128(u.v512[0], 13)}; break;
    case 14: return {_mm512_bsrli_epi128(u.v512[0], 14)}; break;
    case 15: return {_mm512_bsrli_epi128(u.v512[0], 15)}; break;
    case 16: return {_mm512_bsrli_epi128(u.v512[0], 16)}; break;
    case 17: return {_mm512_bsrli_epi128(u.v512[0], 17)}; break;
    case 18: return {_mm512_bsrli_epi128(u.v512[0], 18)}; break;
    case 19: return {_mm512_bsrli_epi128(u.v512[0], 19)}; break;
    case 20: return {_mm512_bsrli_epi128(u.v512[0], 20)}; break;
    case 21: return {_mm512_bsrli_epi128(u.v512[0], 21)}; break;
    case 22: return {_mm512_bsrli_epi128(u.v512[0], 22)}; break;
    case 23: return {_mm512_bsrli_epi128(u.v512[0], 23)}; break;
    case 24: return {_mm512_bsrli_epi128(u.v512[0], 24)}; break;
    case 25: return {_mm512_bsrli_epi128(u.v512[0], 25)}; break;
    case 26: return {_mm512_bsrli_epi128(u.v512[0], 26)}; break;
    case 27: return {_mm512_bsrli_epi128(u.v512[0], 27)}; break;
    case 28: return {_mm512_bsrli_epi128(u.v512[0], 28)}; break;
    case 29: return {_mm512_bsrli_epi128(u.v512[0], 29)}; break;
    case 30: return {_mm512_bsrli_epi128(u.v512[0], 30)}; break;
    case 31: return {_mm512_bsrli_epi128(u.v512[0], 31)}; break;
    case 32: return {_mm512_bsrli_epi128(u.v512[0], 32)}; break;
    case 33: return {_mm512_bsrli_epi128(u.v512[0], 33)}; break;
    case 34: return {_mm512_bsrli_epi128(u.v512[0], 34)}; break;
    case 35: return {_mm512_bsrli_epi128(u.v512[0], 35)}; break;
    case 36: return {_mm512_bsrli_epi128(u.v512[0], 36)}; break;
    case 37: return {_mm512_bsrli_epi128(u.v512[0], 37)}; break;
    case 38: return {_mm512_bsrli_epi128(u.v512[0], 38)}; break;
    case 39: return {_mm512_bsrli_epi128(u.v512[0], 39)}; break;
    case 40: return {_mm512_bsrli_epi128(u.v512[0], 40)}; break;
    case 41: return {_mm512_bsrli_epi128(u.v512[0], 41)}; break;
    case 42: return {_mm512_bsrli_epi128(u.v512[0], 42)}; break;
    case 43: return {_mm512_bsrli_epi128(u.v512[0], 43)}; break;
    case 44: return {_mm512_bsrli_epi128(u.v512[0], 44)}; break;
    case 45: return {_mm512_bsrli_epi128(u.v512[0], 45)}; break;
    case 46: return {_mm512_bsrli_epi128(u.v512[0], 46)}; break;
    case 47: return {_mm512_bsrli_epi128(u.v512[0], 47)}; break;
    case 48: return {_mm512_bsrli_epi128(u.v512[0], 48)}; break;
    case 49: return {_mm512_bsrli_epi128(u.v512[0], 49)}; break;
    case 50: return {_mm512_bsrli_epi128(u.v512[0], 50)}; break;
    case 51: return {_mm512_bsrli_epi128(u.v512[0], 51)}; break;
    case 52: return {_mm512_bsrli_epi128(u.v512[0], 52)}; break;
    case 53: return {_mm512_bsrli_epi128(u.v512[0], 53)}; break;
    case 54: return {_mm512_bsrli_epi128(u.v512[0], 54)}; break;
    case 55: return {_mm512_bsrli_epi128(u.v512[0], 55)}; break;
    case 56: return {_mm512_bsrli_epi128(u.v512[0], 56)}; break;
    case 57: return {_mm512_bsrli_epi128(u.v512[0], 57)}; break;
    case 58: return {_mm512_bsrli_epi128(u.v512[0], 58)}; break;
    case 59: return {_mm512_bsrli_epi128(u.v512[0], 59)}; break;
    case 60: return {_mm512_bsrli_epi128(u.v512[0], 60)}; break;
    case 61: return {_mm512_bsrli_epi128(u.v512[0], 61)}; break;
    case 62: return {_mm512_bsrli_epi128(u.v512[0], 62)}; break;
    case 63: return {_mm512_bsrli_epi128(u.v512[0], 63)}; break;
    case 64: return Zeroes();
    default: break;
    }
    return *this;
}
#endif

#endif // HAVE_AVX512

#endif // SIMD_IMPL_HPP
