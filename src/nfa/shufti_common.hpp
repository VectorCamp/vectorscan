/*
 * Copyright (c) 2015-2017, Intel Corporation
 * Copyright (c) 2020-2021, VectorCamp PC
 * Copyright (c) 2021, Arm Limited
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
 * \brief Shufti: character class acceleration.
 *
 * Utilises the SSSE3 pshufb shuffle instruction
 */

#include "shufti.h"
#include "ue2common.h"
#include "util/arch.h"
#include "util/bitutils.h"
#include "util/unaligned.h"

#include "util/supervector/supervector.hpp"
#include "util/match.hpp"

template <uint16_t S>
static really_inline
const u8 *fwdBlockDouble(SuperVector<S> mask1_lo, SuperVector<S> mask1_hi, SuperVector<S> mask2_lo, SuperVector<S> mask2_hi,
                    SuperVector<S> chars, const u8 *buf) {

    const SuperVector<S> low4bits = SuperVector<S>::dup_u8(0xf);
    SuperVector<S> chars_lo = chars & low4bits;
    chars_lo.print8("chars_lo");
    SuperVector<S> chars_hi = chars.rshift64(4) & low4bits;
    chars_hi.print8("chars_hi");
    SuperVector<S> c1_lo = mask1_lo.pshufb(chars_lo);
    c1_lo.print8("c1_lo");
    SuperVector<S> c1_hi = mask1_hi.pshufb(chars_hi);
    c1_hi.print8("c1_hi");
    SuperVector<S> t1 = c1_lo | c1_hi;
    t1.print8("t1");

    SuperVector<S> c2_lo = mask2_lo.pshufb(chars_lo);
    c2_lo.print8("c2_lo");
    SuperVector<S> c2_hi = mask2_hi.pshufb(chars_hi);
    c2_hi.print8("c2_hi");
    SuperVector<S> t2 = c2_lo | c2_hi;
    t2.print8("t2");
    t2.rshift128(1).print8("t2.rshift128(1)");
    SuperVector<S> t = t1 | (t2.rshift128(1));
    t.print8("t");

    typename SuperVector<S>::movemask_type z = t.eqmask(SuperVector<S>::Ones());
    DEBUG_PRINTF(" z: 0x%016llx\n", (u64a)z);
    return firstMatch<S>(buf, z);
}

template <uint16_t S>
static really_inline const u8 *shuftiDoubleMini(SuperVector<S> mask1_lo, SuperVector<S> mask1_hi, SuperVector<S> mask2_lo, SuperVector<S> mask2_hi,
                       const u8 *buf, const u8 *buf_end){
    uintptr_t len = buf_end - buf;
    assert(len < S);

    const SuperVector<S> low4bits = SuperVector<S>::dup_u8(0xf);

    DEBUG_PRINTF("buf %p buf_end %p \n", buf, buf_end);
    SuperVector<S> chars = SuperVector<S>::loadu_maskz(buf, len);
    chars.print8("chars");

    SuperVector<S> chars_lo = chars & low4bits;
    chars_lo.print8("chars_lo");
    SuperVector<S> chars_hi = chars.rshift64(4) & low4bits;
    chars_hi.print8("chars_hi");
    SuperVector<S> c1_lo = mask1_lo.pshufb_maskz(chars_lo, len);
    c1_lo.print8("c1_lo");
    SuperVector<S> c1_hi = mask1_hi.pshufb_maskz(chars_hi, len);
    c1_hi.print8("c1_hi");
    SuperVector<S> t1 = c1_lo | c1_hi;
    t1.print8("t1");

    SuperVector<S> c2_lo = mask2_lo.pshufb_maskz(chars_lo, len);
    c2_lo.print8("c2_lo");
    SuperVector<S> c2_hi = mask2_hi.pshufb_maskz(chars_hi, len);
    c2_hi.print8("c2_hi");
    SuperVector<S> t2 = c2_lo | c2_hi;
    t2.print8("t2");
    t2.rshift128(1).print8("t2.rshift128(1)");
    SuperVector<S> t = t1 | (t2.rshift128(1));
    t.print8("t");

    typename SuperVector<S>::movemask_type z = t.eqmask(SuperVector<S>::Ones());
    DEBUG_PRINTF(" z: 0x%016llx\n", (u64a)z);
    return firstMatch<S>(buf, z);
}

template <uint16_t S>
const u8 *shuftiDoubleExecReal(m128 mask1_lo, m128 mask1_hi, m128 mask2_lo, m128 mask2_hi,
                           const u8 *buf, const u8 *buf_end) {
        assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("shufti %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const SuperVector<S> wide_mask1_lo(mask1_lo);
    const SuperVector<S> wide_mask1_hi(mask1_hi);
    const SuperVector<S> wide_mask2_lo(mask2_lo);
    const SuperVector<S> wide_mask2_hi(mask2_hi);

    const u8 *d = buf;
    const u8 *rv;

    DEBUG_PRINTF("start %p end %p \n", d, buf_end);
    assert(d < buf_end);
    if (d + S <= buf_end) {
        // peel off first part to cacheline boundary
        const u8 *d1 = ROUNDUP_PTR(d, S);
        DEBUG_PRINTF("until aligned %p \n", d1);
        if (d1 != d) {
            SuperVector<S> chars = SuperVector<S>::loadu(d);
            rv = fwdBlockDouble(wide_mask1_lo, wide_mask1_hi, wide_mask2_lo, wide_mask2_hi, chars, d);
            DEBUG_PRINTF("rv %p \n", rv);
            if (rv) return rv;
            d = d1;
        }

        size_t loops = (buf_end - d) / S;
        DEBUG_PRINTF("loops %ld \n", loops);

        for (size_t i = 0; i < loops; i++, d+= S) {
            DEBUG_PRINTF("it = %ld, d %p \n", i, d);
            const u8 *base = ROUNDUP_PTR(d, S);
            // On large packet buffers, this prefetch appears to get us about 2%.
            __builtin_prefetch(base + 256);

            SuperVector<S> chars = SuperVector<S>::load(d);
            rv = fwdBlockDouble(wide_mask1_lo, wide_mask1_hi, wide_mask2_lo, wide_mask2_hi, chars, d);
            if (rv) return rv;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        rv = shuftiDoubleMini(wide_mask1_lo, wide_mask1_hi, wide_mask2_lo, wide_mask2_hi, d, buf_end);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv >= buf && rv < buf_end) return rv;
    }
    
    return buf_end;
}

const u8 *shuftiDoubleExec(m128 mask1_lo, m128 mask1_hi,
                            m128 mask2_lo, m128 mask2_hi,
                            const u8 *buf, const u8 *buf_end) {
    return shuftiDoubleExecReal<VECTORSIZE>(mask1_lo, mask1_hi, mask2_lo, mask2_hi, buf, buf_end);
}