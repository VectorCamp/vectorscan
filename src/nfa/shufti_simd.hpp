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
typename SuperVector<S>::movemask_type block(SuperVector<S> mask_lo, SuperVector<S> mask_hi,
            SuperVector<S> chars) {
    const SuperVector<S> low4bits = SuperVector<S>::dup_u8(0xf);

    SuperVector<S> c_lo = chars & low4bits;
    c_lo = mask_lo.pshufb(c_lo);
    SuperVector<S> c_hi = mask_hi.pshufb(chars.rshift64(4) & low4bits);
    SuperVector<S> t = c_lo & c_hi;

    return t.eqmask(SuperVector<S>::Zeroes());
}

template <uint16_t S>
static really_inline
const u8 *fwdBlock(SuperVector<S> mask_lo, SuperVector<S> mask_hi, SuperVector<S> chars, const u8 *buf) {
    typename SuperVector<S>::movemask_type z = block(mask_lo, mask_hi, chars);
    DEBUG_PRINTF(" z: 0x%016llx\n", (u64a)z);

    return firstMatch<S>(buf, z);
}
/*
template <uint16_t S>
static really_inline
const u8 *shortShufti(SuperVector<S> mask_lo, SuperVector<S> mask_hi, const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("short shufti %p len %zu\n", buf, buf_end - buf);
    uintptr_t len = buf_end - buf;
    assert(len <= S);

    SuperVector<S> chars = SuperVector<S>::loadu_maskz(buf, static_cast<uint8_t>(len));
    //printv_u8("chars", chars);
    uint8_t alignment = (uintptr_t)(buf) & 15;
    typename SuperVector<S>::movemask_type maskb = 1 << alignment;
    typename SuperVector<S>::movemask_type maske = SINGLE_LOAD_MASK(len - alignment);
    typename SuperVector<S>::movemask_type z = block(mask_lo, mask_hi, chars);
    // reuse the load mask to indicate valid bytes
    DEBUG_PRINTF(" z: 0x%016llx\n", (u64a)z);
    z &= maskb | maske;
    DEBUG_PRINTF(" z: 0x%016llx\n", (u64a)z);

    return firstMatch<S>(buf, z);
}*/

template <uint16_t S>
static really_inline
const u8 *revBlock(SuperVector<S> mask_lo, SuperVector<S> mask_hi, SuperVector<S> chars, const u8 *buf) {
    typename SuperVector<S>::movemask_type z = block(mask_lo, mask_hi, chars);
    DEBUG_PRINTF(" z: 0x%016llx\n", (u64a)z);
    return lastMatch<S>(buf, z);
}

template <uint16_t S>
const u8 *shuftiExecReal(m128 mask_lo, m128 mask_hi, const u8 *buf, const u8 *buf_end) {
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("shufti %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const SuperVector<S> wide_mask_lo(mask_lo);
    const SuperVector<S> wide_mask_hi(mask_hi);

    const u8 *d = buf;
    const u8 *rv;

    DEBUG_PRINTF("start %p end %p \n", d, buf_end);
    assert(d < buf_end);
    if (d + S <= buf_end) {
        // peel off first part to cacheline boundary
        const u8 *d1 = ROUNDUP_PTR(d, S);
        DEBUG_PRINTF("until aligned %p \n", d1);
        if (d1 != d) {
            rv = shuftiFwdSlow((const u8 *)&mask_lo, (const u8 *)&mask_hi, d, d1);
            // rv = shortShufti(wide_mask_lo, wide_mask_hi, d, d1);
            if (rv != d1) {
                return rv;
            }
            d = d1;
        }

        size_t loops = (buf_end - d) / S;
        DEBUG_PRINTF("loops %ld \n", loops);

        for (size_t i = 0; i < loops; i++, d+= S) {
            DEBUG_PRINTF("d %p \n", d);
            const u8 *base = ROUNDUP_PTR(d, S);
            // On large packet buffers, this prefetch appears to get us about 2%.
            __builtin_prefetch(base + 256);

            SuperVector<S> chars = SuperVector<S>::load(d);
            rv = fwdBlock(wide_mask_lo, wide_mask_hi, chars, d);
            if (rv) return rv;
        }
    }

    DEBUG_PRINTF("d %p e %p \n", d, buf_end);
    // finish off tail

    rv = buf_end;
    if (d != buf_end) {
        rv = shuftiFwdSlow((const u8 *)&mask_lo, (const u8 *)&mask_hi, d, buf_end);
        // rv = shortShufti(wide_mask_lo, wide_mask_hi, buf_end - S, buf_end);
        DEBUG_PRINTF("rv %p \n", rv);
    }

    return rv;
}

template <uint16_t S>
const u8 *rshuftiExecReal(m128 mask_lo, m128 mask_hi, const u8 *buf, const u8 *buf_end) {
    assert(buf && buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("shufti %p len %zu\n", buf, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);

    const SuperVector<S> wide_mask_lo(mask_lo);
    const SuperVector<S> wide_mask_hi(mask_hi);

    const u8 *d = buf_end;
    const u8 *rv;

    DEBUG_PRINTF("start %p end %p \n", buf, d);
    assert(d > buf);
    if (d - S >= buf) {
        // peel off first part to cacheline boundary
        const u8 *d1 = ROUNDDOWN_PTR(d, S);
        DEBUG_PRINTF("until aligned %p \n", d1);
        if (d1 != d) {
            rv = shuftiRevSlow((const u8 *)&mask_lo, (const u8 *)&mask_hi, d1, d);
            DEBUG_PRINTF("rv %p \n", rv);
            // rv = shortShufti(wide_mask_lo, wide_mask_hi, d, d1);
            if (rv != d1 - 1) return rv;
            d = d1;
        }

        while (d - S >= buf) {
            DEBUG_PRINTF("aligned %p \n", d);
            d -= S;
            const u8 *base = ROUNDDOWN_PTR(buf, S);
            // On large packet buffers, this prefetch appears to get us about 2%.
            __builtin_prefetch(base + 256);

            SuperVector<S> chars = SuperVector<S>::load(d);
            rv = revBlock(wide_mask_lo, wide_mask_hi, chars, d);
            if (rv) return rv;
        }
    }

    DEBUG_PRINTF("tail d %p e %p \n", buf, d);
    // finish off tail

    if (d != buf) {
        rv = shuftiRevSlow((const u8 *)&mask_lo, (const u8 *)&mask_hi, buf, d);
        // rv = shortShufti(wide_mask_lo, wide_mask_hi, buf_end - S, buf_end);
        DEBUG_PRINTF("rv %p \n", rv);
        if (rv) return rv;
    }

    return buf - 1;
}

const u8 *shuftiExec(m128 mask_lo, m128 mask_hi, const u8 *buf,
                      const u8 *buf_end) {
    return shuftiExecReal<VECTORSIZE>(mask_lo, mask_hi, buf, buf_end);
}

const u8 *rshuftiExec(m128 mask_lo, m128 mask_hi, const u8 *buf,
                       const u8 *buf_end) {
    return rshuftiExecReal<VECTORSIZE>(mask_lo, mask_hi, buf, buf_end);
}
