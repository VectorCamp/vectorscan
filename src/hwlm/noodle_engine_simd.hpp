/*
 * Copyright (c) 2017, Intel Corporation
 * Copyright (c) 2020-2023, VectorCamp PC
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

/* SIMD engine agnostic noodle scan parts */

#include "util/supervector/supervector.hpp"
#include "util/supervector/casemask.hpp"

template <uint16_t S>
static really_really_inline
hwlm_error_t single_zscan(const struct noodTable *n,const u8 *d, const u8 *buf,
                          typename SuperVector<S>::comparemask_type z, size_t len, const struct cb_info *cbi) {
    while (unlikely(z)) {
        typename SuperVector<S>::comparemask_type pos = SuperVector<S>::findLSB(z);
        size_t matchPos = d - buf + pos;
        DEBUG_PRINTF("match pos %zu\n", matchPos);
        hwlmcb_rv_t rv = final(n, buf, len, n->msk_len != 1, cbi, matchPos);
        RETURN_IF_TERMINATED(rv);
    }
    return HWLM_SUCCESS;
}

template <uint16_t S>
static really_really_inline
hwlm_error_t double_zscan(const struct noodTable *n,const u8 *d, const u8 *buf,
                          typename SuperVector<S>::comparemask_type z, size_t len, const struct cb_info *cbi) {
    while (unlikely(z)) {
        typename SuperVector<S>::comparemask_type pos = SuperVector<S>::findLSB(z);
        size_t matchPos = d - buf + pos - 1;
        DEBUG_PRINTF("match pos %zu\n", matchPos);
        hwlmcb_rv_t rv = final(n, buf, len, true, cbi, matchPos);
        RETURN_IF_TERMINATED(rv);
    }
    return HWLM_SUCCESS;
}

template <uint16_t S>
static really_inline
hwlm_error_t scanSingleMain(const struct noodTable *n, const u8 *buf,
                            size_t len, size_t offset,
                            SuperVector<S> caseMask, SuperVector<S> mask1,
                            const struct cb_info *cbi) {
    size_t start = offset + n->msk_len - 1;

    const u8 *d = buf + start;
    const u8 *buf_end = buf + len;
    assert(d < buf_end);

    DEBUG_PRINTF("noodle %p start %zu len %zu\n", buf, start, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);
    DEBUG_PRINTF("start %p end %p \n", d, buf_end);

    __builtin_prefetch(d + 16*64);
    assert(d < buf_end);
    if (d + S <= buf_end) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p, S: %d \n", ROUNDUP_PTR(d, S), S);
        if (!ISALIGNED_N(d, S)) {
            const u8 *d0 = ROUNDDOWN_PTR(d, S);
            DEBUG_PRINTF("d - d0: %ld \n", d - d0);
#if defined(HAVE_MASKED_LOADS)
            uint8_t l = d - d0;
            typename SuperVector<S>::comparemask_type mask = ~SuperVector<S>::load_mask(l);
            SuperVector<S> chars = SuperVector<S>::loadu_maskz(d0, mask) & caseMask;
            typename SuperVector<S>::comparemask_type z = mask1.eqmask(chars);
            DEBUG_PRINTF("mask: %08llx\n", mask);
            hwlm_error_t rv = single_zscan<S>(n, d0, buf, z, len, cbi);
#else
            uint8_t l = d0 + S - d;
            DEBUG_PRINTF("l: %d \n", l);
            SuperVector<S> chars = SuperVector<S>::loadu_maskz(d, l) & caseMask;
            chars.print8("chars");
            typename SuperVector<S>::comparemask_type z = mask1.eqmask(chars);
            DEBUG_PRINTF("z: %08llx\n", (u64a) z);
            z = SuperVector<S>::iteration_mask(z);
            DEBUG_PRINTF("z: %08llx\n", (u64a) z);

            hwlm_error_t rv = single_zscan<S>(n, d, buf, z, len, cbi);
#endif
            chars.print32("chars");
            DEBUG_PRINTF("z: %08llx\n", (u64a) z);

            RETURN_IF_TERMINATED(rv);
            d = d0 + S;
        }

        while(d + S <= buf_end) {
            __builtin_prefetch(d + 16*64);
            DEBUG_PRINTF("d %p \n", d);

            SuperVector<S> v = SuperVector<S>::load(d) & caseMask;
            typename SuperVector<S>::comparemask_type z = mask1.eqmask(v);
            z = SuperVector<S>::iteration_mask(z);

            hwlm_error_t rv = single_zscan<S>(n, d, buf, z, len, cbi);
            RETURN_IF_TERMINATED(rv);
            d += S;
        }
    }

    DEBUG_PRINTF("d %p e %p \n", d, buf_end);
    // finish off tail

    if (d != buf_end) {
        uint8_t l = buf_end - d;
        SuperVector<S> chars = SuperVector<S>::loadu_maskz(d, l) & caseMask;
        typename SuperVector<S>::comparemask_type z = mask1.eqmask(chars);
        z = SuperVector<S>::iteration_mask(z);

        hwlm_error_t rv = single_zscan<S>(n, d, buf, z, len, cbi);
        RETURN_IF_TERMINATED(rv);
    }

    return HWLM_SUCCESS;
}

template <uint16_t S>
static really_inline
hwlm_error_t scanDoubleMain(const struct noodTable *n, const u8 *buf,
                            size_t len, size_t offset,
                            SuperVector<S> caseMask, SuperVector<S> mask1, SuperVector<S> mask2,
                            const struct cb_info *cbi) {
    size_t end = len - n->key_offset + 2;
    size_t start = offset + n->msk_len - n->key_offset;

    const u8 *d = buf + start;
    const u8 *buf_end = buf + end;
    assert(d < buf_end);

    DEBUG_PRINTF("noodle %p start %zu len %zu\n", buf, start, buf_end - buf);
    DEBUG_PRINTF("b %s\n", buf);
    DEBUG_PRINTF("start %p end %p \n", d, buf_end);

    typename SuperVector<S>::comparemask_type lastz1{0};

    __builtin_prefetch(d + 16*64);
    assert(d < buf_end);
    if (d + S <= buf_end) {
        // Reach vector aligned boundaries
        DEBUG_PRINTF("until aligned %p, S: %d \n", ROUNDUP_PTR(d, S), S);
        if (!ISALIGNED_N(d, S)) {
            const u8 *d0 = ROUNDDOWN_PTR(d, S);
#if defined(HAVE_MASKED_LOADS)
            uint8_t l = d - d0;
            typename SuperVector<S>::comparemask_type mask = ~SuperVector<S>::load_mask(l);
            SuperVector<S> chars = SuperVector<S>::loadu_maskz(d0, mask) & caseMask;
            typename SuperVector<S>::comparemask_type z1 = mask1.eqmask(chars);
            typename SuperVector<S>::comparemask_type z2 = mask2.eqmask(chars);
            typename SuperVector<S>::comparemask_type z = (z1 << SuperVector<S>::mask_width()) & z2;
            z = SuperVector<S>::iteration_mask(z);
            lastz1 = z1 >> (S - 1);

            DEBUG_PRINTF("mask: %08llx\n", mask);
            hwlm_error_t rv = double_zscan<S>(n, d0, buf, z, len, cbi);
#else
            uint8_t l = d0 + S - d;
            SuperVector<S> chars = SuperVector<S>::loadu_maskz(d, l) & caseMask;
            chars.print8("chars");
            typename SuperVector<S>::comparemask_type z1 = mask1.eqmask(chars);
            typename SuperVector<S>::comparemask_type z2 = mask2.eqmask(chars);
            typename SuperVector<S>::comparemask_type z = (z1 << SuperVector<S>::mask_width()) & z2;
            z = SuperVector<S>::iteration_mask(z);

            hwlm_error_t rv = double_zscan<S>(n, d, buf, z, len, cbi);
            lastz1 = z1 >> (l - 1);
#endif
            RETURN_IF_TERMINATED(rv);
            d = d0 + S;
        }

        while(d + S <= buf_end) {
            __builtin_prefetch(d + 16*64);
            DEBUG_PRINTF("d %p \n", d);

            SuperVector<S> chars = SuperVector<S>::load(d) & caseMask;
            typename SuperVector<S>::comparemask_type z1 = mask1.eqmask(chars);
            typename SuperVector<S>::comparemask_type z2 = mask2.eqmask(chars);
            typename SuperVector<S>::comparemask_type z = (z1 << SuperVector<S>::mask_width() | lastz1) & z2;
            lastz1 = z1 >> (S - 1);
            z = SuperVector<S>::iteration_mask(z);

            hwlm_error_t rv = double_zscan<S>(n, d, buf, z, len, cbi);
            RETURN_IF_TERMINATED(rv);
            d += S;
        }
    }

    DEBUG_PRINTF("d %p e %p \n", d, buf_end);
    // finish off tail
    if (d != buf_end) {
        uint8_t l = buf_end - d;
        SuperVector<S> chars = SuperVector<S>::loadu_maskz(d, l) & caseMask;
        typename SuperVector<S>::comparemask_type z1 = mask1.eqmask(chars);
        typename SuperVector<S>::comparemask_type z2 = mask2.eqmask(chars);
        typename SuperVector<S>::comparemask_type z = (z1 << SuperVector<S>::mask_width() | lastz1) & z2;
        z = SuperVector<S>::iteration_mask(z);

        hwlm_error_t rv = double_zscan<S>(n, d, buf, z, len, cbi);
        RETURN_IF_TERMINATED(rv);
    }

    return HWLM_SUCCESS;
}

// Single-character specialisation, used when keyLen = 1
static really_inline
hwlm_error_t scanSingle(const struct noodTable *n, const u8 *buf, size_t len,
                        size_t start, bool noCase, const struct cb_info *cbi) {
    /*
     * TODO: Investigate adding scalar case for smaller sizes
    if (len < VECTORSIZE) {
      return scanSingleSlow(n, buf, len, start, noCase, n->key0, cbi);
    }*/

    if (!ourisalpha(n->key0)) {
        noCase = 0; // force noCase off if we don't have an alphabetic char
    }

    const SuperVector<VECTORSIZE> caseMask{noCase ? getCaseMask<VECTORSIZE>() : SuperVector<VECTORSIZE>::Ones()};
    const SuperVector<VECTORSIZE> mask1{getMask<VECTORSIZE>(n->key0, noCase)};

    return scanSingleMain(n, buf, len, start, caseMask, mask1, cbi);
}


static really_inline
hwlm_error_t scanDouble(const struct noodTable *n, const u8 *buf, size_t len,
                        size_t start, bool noCase, const struct cb_info *cbi) {

    const SuperVector<VECTORSIZE> caseMask{noCase ? getCaseMask<VECTORSIZE>() : SuperVector<VECTORSIZE>::Ones()};
    const SuperVector<VECTORSIZE> mask1{getMask<VECTORSIZE>(n->key0, noCase)};
    const SuperVector<VECTORSIZE> mask2{getMask<VECTORSIZE>(n->key1, noCase)};

    return scanDoubleMain(n, buf, len, start, caseMask, mask1, mask2, cbi);
}
