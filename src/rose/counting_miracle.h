/*
 * Copyright (c) 2015-2017, Intel Corporation
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

#ifndef ROSE_COUNTING_MIRACLE_H
#define ROSE_COUNTING_MIRACLE_H

#include "ue2common.h"
#include "runtime.h"
#include "rose_internal.h"
#include "nfa/nfa_api_queue.h"
#include "util/simd_utils.h"

/** \brief Maximum number of bytes to scan when looking for a "counting miracle"
 * stop character. */
#define COUNTING_MIRACLE_LEN_MAX 256

#ifdef HAVE_SVE2

static really_inline
size_t countMatches(svuint8_t chars, svbool_t pg, const u8 *buf) {
    svuint8_t vec = svld1_u8(pg, buf);
    return svcntp_b8(svptrue_b8(), svmatch(pg, vec, chars));
}

static really_inline
bool countLoopBody(svuint8_t chars, svbool_t pg, const u8 *d,
                   u32 target_count, u32 *count_inout, const u8 **d_out) {
    *count_inout += countMatches(chars, pg, d);
    if (*count_inout >= target_count) {
        *d_out = d;
        return true;
    }
    return false;
}

static really_inline
bool countOnce(svuint8_t chars, const u8 *d, const u8 *d_end,
               u32 target_count, u32 *count_inout, const u8 **d_out) {
    assert(d <= d_end);
    svbool_t pg = svwhilelt_b8_s64(0, d_end - d);
    return countLoopBody(chars, pg, d, target_count, count_inout, d_out);
}

static really_inline
bool roseCountingMiracleScan(u8 c, const u8 *d, const u8 *d_end,
                             u32 target_count, u32 *count_inout,
                             const u8 **d_out) {
    assert(d <= d_end);
    svuint8_t chars = svdup_u8(c);
    size_t len = d_end - d;
    if (len <= svcntb()) {
        bool rv = countOnce(chars, d, d_end, target_count, count_inout, d_out);
        return rv;
    }
    // peel off first part to align to the vector size
    const u8 *aligned_d_end = ROUNDDOWN_PTR(d_end, svcntb_pat(SV_POW2));
    assert(d < aligned_d_end);
    if (d_end != aligned_d_end) {
        if (countOnce(chars, aligned_d_end, d_end,
                      target_count, count_inout, d_out)) return true;
        d_end = aligned_d_end;
    }
    size_t loops = (d_end - d) / svcntb();
    for (size_t i = 0; i < loops; i++) {
        d_end -= svcntb();
        if (countLoopBody(chars, svptrue_b8(), d_end,
                          target_count, count_inout, d_out)) return true;
    }
    if (d != d_end) {
        if (countOnce(chars, d, d_end,
                      target_count, count_inout, d_out)) return true;
    }
    return false;
}

#else

static really_inline
char roseCountingMiracleScan(u8 c, const u8 *d, const u8 *d_end,
                             u32 target_count, u32 *count_inout,
                             const u8 **d_out) {
    assert(d <= d_end);

    u32 count = *count_inout;

    m128 chars = set1_16x8(c);

    for (; d + 16 <= d_end; d_end -= 16) {
        m128 data = loadu128(d_end - 16);
        u32 z1 = movemask128(eq128(chars, data));
        count += popcount32(z1);

        if (count >= target_count) {
            *d_out = d_end - 16;
            *count_inout = count;
            return 1;
        }
    }

    if (d != d_end) {
        char temp[sizeof(m128)];
        assert(d + sizeof(temp) > d_end);
        memset(temp, c + 1, sizeof(temp));
        memcpy(temp, d, d_end - d);
        m128 data = loadu128(temp);
        u32 z1 = movemask128(eq128(chars, data));
        count += popcount32(z1);

        if (count >= target_count) {
            *d_out = d;
            *count_inout = count;
            return 1;
        }
    }

    *count_inout = count;
    return 0;
}

#endif

#ifdef HAVE_SVE

static really_inline
size_t countShuftiMatches(svuint8_t mask_lo, svuint8_t mask_hi,
                          const svbool_t pg, const u8 *buf) {
    svuint8_t vec = svld1_u8(pg, buf);
    svuint8_t c_lo = svtbl(mask_lo, svand_z(svptrue_b8(), vec, (uint8_t)0xf));
    svuint8_t c_hi = svtbl(mask_hi, svlsr_z(svptrue_b8(), vec, 4));
    svuint8_t t = svand_z(svptrue_b8(), c_lo, c_hi);
    return svcntp_b8(svptrue_b8(), svcmpne(pg, t, (uint8_t)0));
}

static really_inline
bool countShuftiLoopBody(svuint8_t mask_lo, svuint8_t mask_hi,
                         const svbool_t pg, const u8 *d, u32 target_count,
                         u32 *count_inout, const u8 **d_out) {
    *count_inout += countShuftiMatches(mask_lo, mask_hi, pg, d);
    if (*count_inout >= target_count) {
        *d_out = d;
        return true;
    }
    return false;
}

static really_inline
bool countShuftiOnce(svuint8_t mask_lo, svuint8_t mask_hi,
                     const u8 *d, const u8 *d_end, u32 target_count,
                     u32 *count_inout, const u8 **d_out) {
    svbool_t pg = svwhilelt_b8_s64(0, d_end - d);
    return countShuftiLoopBody(mask_lo, mask_hi, pg, d, target_count,
                               count_inout, d_out);
}

static really_inline
bool roseCountingMiracleScanShufti(svuint8_t mask_lo, svuint8_t mask_hi,
                                   UNUSED u8 poison, const u8 *d,
                                   const u8 *d_end, u32 target_count,
                                   u32 *count_inout, const u8 **d_out) {
    assert(d <= d_end);
    size_t len = d_end - d;
    if (len <= svcntb()) {
        char rv = countShuftiOnce(mask_lo, mask_hi, d, d_end, target_count,
                                  count_inout, d_out);
        return rv;
    }
    // peel off first part to align to the vector size
    const u8 *aligned_d_end = ROUNDDOWN_PTR(d_end, svcntb_pat(SV_POW2));
    assert(d < aligned_d_end);
    if (d_end != aligned_d_end) {
        if (countShuftiOnce(mask_lo, mask_hi, aligned_d_end, d_end,
                            target_count, count_inout, d_out)) return true;
        d_end = aligned_d_end;
    }
    size_t loops = (d_end - d) / svcntb();
    for (size_t i = 0; i < loops; i++) {
        d_end -= svcntb();
        if (countShuftiLoopBody(mask_lo, mask_hi, svptrue_b8(), d_end,
                                target_count, count_inout, d_out)) return true;
    }
    if (d != d_end) {
        if (countShuftiOnce(mask_lo, mask_hi, d, d_end,
                            target_count, count_inout, d_out)) return true;
    }
    return false;
}

#else

#define GET_LO_4(chars) and128(chars, low4bits)
#define GET_HI_4(chars) rshift64_m128(andnot128(low4bits, chars), 4)

static really_inline
u32 roseCountingMiracleScanShufti(m128 mask_lo, m128 mask_hi, u8 poison,
                                  const u8 *d, const u8 *d_end,
                                  u32 target_count, u32 *count_inout,
                                  const u8 **d_out) {
    assert(d <= d_end);

    u32 count = *count_inout;

    const m128 zeroes = zeroes128();
    const m128 low4bits = set1_16x8(0xf);

    for (; d + 16 <= d_end; d_end -= 16) {
        m128 data = loadu128(d_end - 16);
        m128 c_lo  = pshufb_m128(mask_lo, GET_LO_4(data));
        m128 c_hi  = pshufb_m128(mask_hi, GET_HI_4(data));
        m128 t     = and128(c_lo, c_hi);
        u32 z1 = movemask128(eq128(t, zeroes));
        count += popcount32(z1 ^ 0xffff);

        if (count >= target_count) {
            *d_out = d_end - 16;
            *count_inout = count;
            return 1;
        }
    }

    if (d != d_end) {
        char temp[sizeof(m128)];
        assert(d + sizeof(temp) > d_end);
        memset(temp, poison, sizeof(temp));
        memcpy(temp, d, d_end - d);
        m128 data  = loadu128(temp);
        m128 c_lo  = pshufb_m128(mask_lo, GET_LO_4(data));
        m128 c_hi  = pshufb_m128(mask_hi, GET_HI_4(data));
        m128 t     = and128(c_lo, c_hi);
        u32 z1 = movemask128(eq128(t, zeroes));
        count += popcount32(z1 ^ 0xffff);

        if (count >= target_count) {
            *d_out = d;
            *count_inout = count;
            return 1;
        }
    }

    *count_inout = count;
    return 0;
}

#endif

/**
 * \brief "Counting Miracle" scan: If we see more than N instances of a
 * particular character class we know that the engine must be dead.
 *
 * Scans the buffer/history between relative locations \a begin_loc and \a
 * end_loc, and returns a miracle location (if any) that appears in the stream
 * after \a begin_loc.
 *
 * Returns 1 if some bytes can be skipped and sets \a miracle_loc
 * appropriately, 0 otherwise.
 */
static never_inline
int roseCountingMiracleOccurs(const struct RoseEngine *t,
                              const struct LeftNfaInfo *left,
                              const struct core_info *ci, s64a begin_loc,
                              const s64a end_loc, s64a *miracle_loc) {
    if (!left->countingMiracleOffset) {
        return 0;
    }

    const struct RoseCountingMiracle *cm
        = (const void *)((const char *)t + left->countingMiracleOffset);

    assert(!left->transient);
    assert(cm->count > 1); /* should be a normal miracle then */

    DEBUG_PRINTF("looking for counting miracle over [%lld,%lld], maxLag=%u\n",
                 begin_loc, end_loc, left->maxLag);
    DEBUG_PRINTF("ci->len=%zu, ci->hlen=%zu\n", ci->len, ci->hlen);

    assert(begin_loc <= end_loc);
    assert(begin_loc >= -(s64a)ci->hlen);
    assert(end_loc <= (s64a)ci->len);

    const s64a scan_end_loc = end_loc - left->maxLag;
    if (scan_end_loc <= begin_loc) {
        DEBUG_PRINTF("nothing to scan\n");
        return 0;
    }

    const s64a start = MAX(begin_loc, scan_end_loc - COUNTING_MIRACLE_LEN_MAX);
    DEBUG_PRINTF("scan [%lld..%lld]\n", start, scan_end_loc);

    u32 count = 0;

    s64a m_loc = start;

    if (!cm->shufti) {
        u8 c = cm->c;

        // Scan buffer.
        const s64a buf_scan_start = MAX(0, start);
        if (scan_end_loc > buf_scan_start) {
            const u8 *buf = ci->buf;
            const u8 *d = buf + scan_end_loc;
            const u8 *d_start = buf + buf_scan_start;
            const u8 *d_out;
            if (roseCountingMiracleScan(c, d_start, d, cm->count, &count,
                                        &d_out)) {
                assert(d_out >= d_start);
                m_loc = (d_out - d_start) + buf_scan_start;
                goto success;
            }
        }

        // Scan history.
        if (start < 0) {
            const u8 *hbuf_end = ci->hbuf + ci->hlen;
            const u8 *d = hbuf_end + MIN(0, scan_end_loc);
            const u8 *d_start = hbuf_end + start;
            const u8 *d_out;
            if (roseCountingMiracleScan(c, d_start, d, cm->count, &count,
                                        &d_out)) {
                assert(d_out >= d_start);
                m_loc = (d_out - d_start) + start;
                goto success;
            }
        }
    } else {
#ifdef HAVE_SVE
        svuint8_t lo = getSVEMaskFrom128(cm->lo);
        svuint8_t hi = getSVEMaskFrom128(cm->hi);
#else
        m128 lo = cm->lo;
        m128 hi = cm->hi;
#endif
        u8 poison = cm->poison;

        // Scan buffer.
        const s64a buf_scan_start = MAX(0, start);
        if (scan_end_loc > buf_scan_start) {
            const u8 *buf = ci->buf;
            const u8 *d = buf + scan_end_loc;
            const u8 *d_start = buf + buf_scan_start;
            const u8 *d_out;
            if (roseCountingMiracleScanShufti(lo, hi, poison, d_start, d,
                                              cm->count, &count, &d_out)) {
                assert(d_out >= d_start);
                m_loc = (d_out - d_start) + buf_scan_start;
                goto success;
            }
        }

        // Scan history.
        if (start < 0) {
            const u8 *hbuf_end = ci->hbuf + ci->hlen;
            const u8 *d = hbuf_end + MIN(0, scan_end_loc);
            const u8 *d_start = hbuf_end + start;
            const u8 *d_out;
            if (roseCountingMiracleScanShufti(lo, hi, poison, d_start, d,
                                              cm->count, &count, &d_out)) {
                assert(d_out >= d_start);
                m_loc = (d_out - d_start) + start;
                goto success;
            }
        }
    }

    DEBUG_PRINTF("found %u/%u\n", count, cm->count);
    return 0;

success:
    DEBUG_PRINTF("found %u/%u\n", count, cm->count);
    assert(count >= cm->count);
    assert(m_loc < scan_end_loc);
    assert(m_loc >= start);

    *miracle_loc = m_loc;
    return 1;
}

#endif
