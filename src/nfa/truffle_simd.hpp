/*
 * Copyright (c) 2015-2017, Intel Corporation
 * Copyright (c) 2020-2023, VectorCamp PC
 * Copyright (c) 2023, Arm Limited
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
 * \brief Truffle: character class acceleration.
 *
 */

#include "truffle.h"
#include "ue2common.h"
#include "util/arch.h"
#include "util/bitutils.h"
#include "util/unaligned.h"

#include "util/supervector/supervector.hpp"
#include "util/match.hpp"

#ifdef HAVE_SVE
static really_inline
svuint8_t blockSingleMask(svuint8_t shuf_mask_lo_highclear, svuint8_t shuf_mask_lo_highset, svuint8_t chars);

static really_inline
svuint8_t blockSingleMaskWide32(svuint8_t shuf_mask_32, svuint8_t chars);

#ifdef HAVE_SVE2
static really_inline
svuint8_t blockSingleMaskWide(svuint8_t shuf_mask_lo_highclear, svuint8_t shuf_mask_lo_highset, svuint8_t chars);
#endif //HAVE_SVE2
#else
template <uint16_t S>
static really_inline
const SuperVector<S> blockSingleMask(SuperVector<S> shuf_mask_lo_highclear, SuperVector<S> shuf_mask_lo_highset, SuperVector<S> chars);
#endif //HAVE_SVE

#if defined(VS_SIMDE_BACKEND)
#include "x86/truffle.hpp"
#else
#if defined(ARCH_IA32) || defined(ARCH_X86_64)
#include "x86/truffle.hpp"
#elif defined(ARCH_ARM32) || defined(ARCH_AARCH64)
#include "arm/truffle.hpp"
#elif defined(ARCH_PPC64EL)
#include "ppc64el/truffle.hpp"
#endif
#endif

#ifdef HAVE_SVE
template <bool is_wide, bool is_vector_128b>
static really_inline
const u8 *truffleExecSVE(m256 shuf_mask_32,
                      const u8 *buf, const u8 *buf_end);

template <bool is_wide, bool is_vector_128b>
static really_inline
const u8 *rtruffleExecSVE(m256 shuf_mask_32,
                       const u8 *buf, const u8 *buf_end);

template <bool is_wide, bool is_vector_128b>
static really_inline
const u8 *scanBlock(svuint8_t shuf_mask_lo_highclear, svuint8_t shuf_mask_lo_highset,
                    svuint8_t chars, const u8 *buf, const size_t vector_size_int_8, bool forward)
{
    svuint8_t result_mask;
    if(is_wide) {
        if(is_vector_128b) {
#ifdef HAVE_SVE2
            result_mask = blockSingleMaskWide(shuf_mask_lo_highclear, shuf_mask_lo_highset, chars);
#else
            DEBUG_PRINTF("Wide Truffle is not supported with 128b vectors unless SVE2 is enabled");
            assert(false);
#endif
        } else {
            result_mask = blockSingleMaskWide32(shuf_mask_lo_highclear, chars);
        }
    } else {
        result_mask = blockSingleMask(shuf_mask_lo_highclear, shuf_mask_lo_highset, chars);
    }
    uint64_t index;
    if (forward) {
        index = first_non_zero(vector_size_int_8, result_mask);
    } else {
        index = last_non_zero(vector_size_int_8, result_mask);
    }

    if (index < vector_size_int_8) {
        return buf+index;
    } else {
        return NULL;
    }
}

template <bool is_wide, bool is_vector_128b>
static really_inline
const u8 *truffleExecSVE(m256 shuf_mask_32, const u8 *buf, const u8 *buf_end) {
    const int vect_size_int8 = svcntb();
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("truffle %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    svuint8_t wide_shuf_mask_lo_highclear;
    svuint8_t wide_shuf_mask_lo_highset;
    if (is_wide && !is_vector_128b) {
        const svbool_t lane_pred_32 = svwhilelt_b8(0, 32);
        wide_shuf_mask_lo_highclear = svld1(lane_pred_32, (uint8_t*) &shuf_mask_32.lo);
        wide_shuf_mask_lo_highset = svld1(svpfalse(), (uint8_t*) &shuf_mask_32.hi); /* empty vector */
    } else {
        const svbool_t lane_pred_16 = svwhilelt_b8(0, 16);
        wide_shuf_mask_lo_highclear = svld1(lane_pred_16, (uint8_t*) &shuf_mask_32.lo);
        wide_shuf_mask_lo_highset = svld1(lane_pred_16, (uint8_t*) &shuf_mask_32.hi);
    }

    const u8 *work_buffer = buf;
    const u8 *ret_val;

    DEBUG_PRINTF("start %p end %p \n", work_buffer, buf_end);
    assert(work_buffer < buf_end);

    __builtin_prefetch(work_buffer + 16*64);

    if (work_buffer + vect_size_int8 <= buf_end) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p \n", ROUNDUP_PTR(work_buffer, vect_size_int8));
        if (!ISALIGNED_N(work_buffer, vect_size_int8)) {
            svuint8_t chars = svld1(svptrue_b8(), work_buffer);
            const u8 *alligned_buffer = ROUNDUP_PTR(work_buffer, vect_size_int8);
            ret_val = scanBlock<is_wide, is_vector_128b>(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, work_buffer, vect_size_int8, true);
            if (ret_val && ret_val < alligned_buffer) return ret_val;
            work_buffer = alligned_buffer;
        }

        while (work_buffer + vect_size_int8 <= buf_end) {
            __builtin_prefetch(work_buffer + 16*64);
            DEBUG_PRINTF("work_buffer %p \n", work_buffer);
            svuint8_t chars = svld1(svptrue_b8(), work_buffer);
            ret_val = scanBlock<is_wide, is_vector_128b>(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, work_buffer, vect_size_int8, true);
            if (ret_val) return ret_val;
            work_buffer += vect_size_int8;
        }
    }

    DEBUG_PRINTF("work_buffer %p e %p \n", work_buffer, buf_end);
    // finish off tail

    if (work_buffer != buf_end) {
        svuint8_t chars;
        const u8* end_buf;
        if (buf_end - buf < vect_size_int8) {
            const svbool_t remaining_lanes = svwhilelt_b8(0ll, buf_end - buf);
            chars = svld1(remaining_lanes, buf);
            end_buf = buf;
        } else {
            chars = svld1(svptrue_b8(), buf_end - vect_size_int8);
            end_buf = buf_end - vect_size_int8;
        }
        ret_val = scanBlock<is_wide, is_vector_128b>(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, end_buf, vect_size_int8, true);
        DEBUG_PRINTF("ret_val %p \n", ret_val);
        if (ret_val && ret_val < buf_end) return ret_val;
    }

    return buf_end;
}

template <bool is_wide, bool is_vector_128b>
static really_inline
const u8 *rtruffleExecSVE(m256 shuf_mask_32, const u8 *buf, const u8 *buf_end){
    const int vect_size_int8 = svcntb();
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("truffle %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    svuint8_t wide_shuf_mask_lo_highclear;
    svuint8_t wide_shuf_mask_lo_highset;
    if (is_wide && !is_vector_128b) {
        const svbool_t lane_pred_32 = svwhilelt_b8(0, 32);
        wide_shuf_mask_lo_highclear = svld1(lane_pred_32, (uint8_t*) &shuf_mask_32.lo);
        wide_shuf_mask_lo_highset = svld1(svpfalse(), (uint8_t*) &shuf_mask_32.hi); /* empty vector */
    } else {
        const svbool_t lane_pred_16 = svwhilelt_b8(0, 16);
        wide_shuf_mask_lo_highclear = svld1(lane_pred_16, (uint8_t*) &shuf_mask_32.lo);
        wide_shuf_mask_lo_highset = svld1(lane_pred_16, (uint8_t*) &shuf_mask_32.hi);
    }

    const u8 *work_buffer = buf_end;
    const u8 *ret_val;

    DEBUG_PRINTF("start %p end %p \n", buf, work_buffer);
    assert(work_buffer > buf);

    __builtin_prefetch(work_buffer - 16*64);

    if (work_buffer - vect_size_int8 >= buf) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p \n", ROUNDDOWN_PTR(work_buffer, vect_size_int8));
        if (!ISALIGNED_N(work_buffer, vect_size_int8)) {
            svuint8_t chars = svld1(svptrue_b8(), work_buffer - vect_size_int8);
            const u8 *alligned_buffer = ROUNDDOWN_PTR(work_buffer, vect_size_int8);
            ret_val = scanBlock<is_wide, is_vector_128b>(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, work_buffer - vect_size_int8, vect_size_int8, false);
            DEBUG_PRINTF("ret_val %p \n", ret_val);
            if (ret_val >= alligned_buffer) return ret_val;
            work_buffer = alligned_buffer;
        }

        while (work_buffer - vect_size_int8 >= buf) {
            DEBUG_PRINTF("aligned %p \n", work_buffer);
            // On large packet buffers, this prefetch appears to get us about 2%.
            __builtin_prefetch(work_buffer - 16*64);

            work_buffer -= vect_size_int8;
            svuint8_t chars = svld1(svptrue_b8(), work_buffer);
            ret_val = scanBlock<is_wide, is_vector_128b>(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, work_buffer, vect_size_int8, false);
            if (ret_val) return ret_val;
        }
    }

    DEBUG_PRINTF("tail work_buffer %p e %p \n", buf, work_buffer);
    // finish off head

    if (work_buffer != buf) {
        svuint8_t chars;
        if (buf_end - buf < vect_size_int8) {
            const svbool_t remaining_lanes = svwhilele_b8(0ll, buf_end - buf);
            chars = svld1(remaining_lanes, buf);
        } else {
            chars = svld1(svptrue_b8(), buf);
        }
        ret_val = scanBlock<is_wide, is_vector_128b>(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, buf, vect_size_int8, false);
        DEBUG_PRINTF("ret_val %p \n", ret_val);
        if (ret_val && ret_val < buf_end) return ret_val;
    }

    return buf - 1;
}
#else
template <uint16_t S>
static really_inline
const u8 *fwdBlock(SuperVector<S> shuf_mask_lo_highclear, SuperVector<S> shuf_mask_lo_highset, SuperVector<S> chars, const u8 *buf) {
    SuperVector<S> res = blockSingleMask(shuf_mask_lo_highclear, shuf_mask_lo_highset, chars);
    return first_zero_match_inverted<S>(buf, res);
}

template <uint16_t S>
const u8 *truffleExecReal(const m128 &shuf_mask_lo_highclear, m128 shuf_mask_lo_highset, const u8 *buf, const u8 *buf_end) {
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("truffle %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const SuperVector<S> wide_shuf_mask_lo_highclear(shuf_mask_lo_highclear);
    const SuperVector<S> wide_shuf_mask_lo_highset(shuf_mask_lo_highset);

    const u8 *d = buf;
    const u8 *rv;

    __builtin_prefetch(d + 16*64);
    DEBUG_PRINTF("start %p end %p \n", d, buf_end);
    assert(d < buf_end);
    if (d + S <= buf_end) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p \n", ROUNDUP_PTR(d, S));
        if (!ISALIGNED_N(d, S)) {
            SuperVector<S> chars = SuperVector<S>::loadu(d);
            const u8 *dup = ROUNDUP_PTR(d, S);
            rv = fwdBlock(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, d);
            if (rv && rv < dup) return rv;
            d = dup;
        }

        while (d + S <= buf_end) {
            __builtin_prefetch(d + 16*64);
            DEBUG_PRINTF("d %p \n", d);
            SuperVector<S> chars = SuperVector<S>::load(d);
            rv = fwdBlock(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, d);
            if (rv) return rv;
            d += S;
        }
    }

    DEBUG_PRINTF("d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        SuperVector<S> chars = SuperVector<S>::Zeroes();
        const u8* end_buf;
        if (buf_end - buf < S) {
          memcpy(&chars.u, buf, buf_end - buf);
          end_buf = buf;
        } else {
          chars = SuperVector<S>::loadu(buf_end - S);
          end_buf = buf_end - S;
        }
        rv = fwdBlock(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, end_buf);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    return buf_end;
}

template <uint16_t S>
static really_inline
const u8 *revBlock(SuperVector<S> shuf_mask_lo_highclear, SuperVector<S> shuf_mask_lo_highset, SuperVector<S> v, 
                    const u8 *buf) {
    SuperVector<S> res = blockSingleMask(shuf_mask_lo_highclear, shuf_mask_lo_highset, v);
    return last_zero_match_inverted<S>(buf, res);
}

template <uint16_t S>
const u8 *rtruffleExecReal(m128 shuf_mask_lo_highclear, m128 shuf_mask_lo_highset, const u8 *buf, const u8 *buf_end){
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("trufle %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const SuperVector<S> wide_shuf_mask_lo_highclear(shuf_mask_lo_highclear);
    const SuperVector<S> wide_shuf_mask_lo_highset(shuf_mask_lo_highset);

    const u8 *d = buf_end;
    const u8 *rv;

    __builtin_prefetch(d - 16*64);
    DEBUG_PRINTF("start %p end %p \n", buf, d);
    assert(d > buf);
    if (d - S >= buf) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p \n", ROUNDDOWN_PTR(d, S));
        if (!ISALIGNED_N(d, S)) {
            SuperVector<S> chars = SuperVector<S>::loadu(d - S);
            const u8 *dbot = ROUNDDOWN_PTR(d, S);
            rv = revBlock(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, d - S);
            DEBUG_PRINTF("rv %p \n", rv);
            if (rv >= dbot) return rv;
            d = dbot;
        }

        while (d - S >= buf) {
            DEBUG_PRINTF("aligned %p \n", d);
            // On large packet buffers, this prefetch appears to get us about 2%.
            __builtin_prefetch(d - 16*64);

            d -= S;
            SuperVector<S> chars = SuperVector<S>::load(d);
            rv = revBlock(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, d);
            if (rv) return rv;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", buf, d);
    // finish off head

    if (d != buf) {
        SuperVector<S> chars = SuperVector<S>::Zeroes();
        if (buf_end - buf < S) {
          memcpy(&chars.u, buf, buf_end - buf);
        } else {
          chars = SuperVector<S>::loadu(buf);
        }
        rv = revBlock(wide_shuf_mask_lo_highclear, wide_shuf_mask_lo_highset, chars, buf);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv && rv < buf_end) return rv;
    }

    return buf - 1;
}
#endif //HAVE_SVE
