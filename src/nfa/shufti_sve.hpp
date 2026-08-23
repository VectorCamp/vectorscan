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

/** \file
 * \brief Shufti: character class acceleration.
 *
 * Utilises the SVE tbl shuffle instruction
 */

static really_inline
svbool_t singleMatched(svuint8_t mask_lo, svuint8_t mask_hi,
                       const u8 *buf, svbool_t pg) {
    svuint8_t vec = svld1_u8(pg, buf);
    svuint8_t c_lo = svtbl(mask_lo, svand_z(svptrue_b8(), vec, (uint8_t)0xf));
    svuint8_t c_hi = svtbl(mask_hi, svlsr_z(svptrue_b8(), vec, 4));
    svuint8_t t = svand_z(svptrue_b8(), c_lo, c_hi);
    return svcmpne(pg, t, (uint8_t)0);
}

static really_inline
const u8 *shuftiOnce(svuint8_t mask_lo, svuint8_t mask_hi,
                     const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("start %p end %p\n", buf, buf_end);
    assert(buf <= buf_end);
    svbool_t pg = svwhilelt_b8_s64(0, buf_end - buf);
    svbool_t matched = singleMatched(mask_lo, mask_hi, buf, pg);
    return accelSearchCheckMatched(buf, matched);
}

static really_inline
const u8 *shuftiLoopBody(svuint8_t mask_lo, svuint8_t mask_hi, const u8 *buf) {
    DEBUG_PRINTF("start %p end %p\n", buf, buf + svcntb());
    svbool_t matched = singleMatched(mask_lo, mask_hi, buf, svptrue_b8());
    return accelSearchCheckMatched(buf, matched);
}

static really_inline
const u8 *rshuftiOnce(svuint8_t mask_lo, svuint8_t mask_hi,
                      const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("start %p end %p\n", buf, buf_end);
    assert(buf <= buf_end);
    DEBUG_PRINTF("l = %td\n", buf_end - buf);
    svbool_t pg = svwhilelt_b8_s64(0, buf_end - buf);
    svbool_t matched = singleMatched(mask_lo, mask_hi, buf, pg);
    return accelRevSearchCheckMatched(buf, matched);
}

static really_inline
const u8 *rshuftiLoopBody(svuint8_t mask_lo, svuint8_t mask_hi,
                          const u8 *buf) {
    svbool_t matched = singleMatched(mask_lo, mask_hi, buf, svptrue_b8());
    return accelRevSearchCheckMatched(buf, matched);
}

static really_inline
const u8 *shuftiSearch(svuint8_t mask_lo, svuint8_t mask_hi,
                       const u8 *buf, const u8 *buf_end) {
    assert(buf < buf_end);
    size_t len = buf_end - buf;
    if (len <= svcntb()) {
        return shuftiOnce(mask_lo, mask_hi, buf, buf_end);
    }
    // peel off first part to align to the vector size
    const u8 *aligned_buf = ROUNDUP_PTR(buf, svcntb_pat(SV_POW2));
    assert(aligned_buf < buf_end);
    if (buf != aligned_buf) {
        const u8 *ptr = shuftiLoopBody(mask_lo, mask_hi, buf);
        if (ptr) return ptr;
    }
    buf = aligned_buf;
    size_t loops = (buf_end - buf) / svcntb();
    DEBUG_PRINTF("loops %zu \n", loops);
    for (size_t i = 0; i < loops; i++, buf += svcntb()) {
        const u8 *ptr = shuftiLoopBody(mask_lo, mask_hi, buf);
        if (ptr) return ptr;
    }
    DEBUG_PRINTF("buf %p buf_end %p \n", buf, buf_end);
    return buf == buf_end ? NULL : shuftiLoopBody(mask_lo, mask_hi,
                                                  buf_end - svcntb());
}

static really_inline
const u8 *rshuftiSearch(svuint8_t mask_lo, svuint8_t mask_hi,
                        const u8 *buf, const u8 *buf_end) {
    assert(buf < buf_end);
    size_t len = buf_end - buf;
    if (len <= svcntb()) {
        return rshuftiOnce(mask_lo, mask_hi, buf, buf_end);
    }
    // peel off first part to align to the vector size
    const u8 *aligned_buf_end = ROUNDDOWN_PTR(buf_end, svcntb_pat(SV_POW2));
    assert(buf < aligned_buf_end);
    if (buf_end != aligned_buf_end) {
        const u8 *ptr = rshuftiLoopBody(mask_lo, mask_hi, buf_end - svcntb());
        if (ptr) return ptr;
    }
    buf_end = aligned_buf_end;
    size_t loops = (buf_end - buf) / svcntb();
    DEBUG_PRINTF("loops %zu \n", loops);
    for (size_t i = 0; i < loops; i++) {
        buf_end -= svcntb();
        const u8 *ptr = rshuftiLoopBody(mask_lo, mask_hi, buf_end);
        if (ptr) return ptr;
    }
    DEBUG_PRINTF("buf %p buf_end %p \n", buf, buf_end);
    return buf == buf_end ? NULL : rshuftiLoopBody(mask_lo, mask_hi, buf);
}

const u8 *shuftiExec(m128 mask_lo, m128 mask_hi, const u8 *buf,
                     const u8 *buf_end) {
    DEBUG_PRINTF("shufti scan over %td bytes\n", buf_end - buf);
    svuint8_t sve_mask_lo = getSVEMaskFrom128(mask_lo);
    svuint8_t sve_mask_hi = getSVEMaskFrom128(mask_hi);
    const u8 *ptr = shuftiSearch(sve_mask_lo, sve_mask_hi, buf, buf_end);
    return ptr ? ptr : buf_end;
}

const u8 *rshuftiExec(m128 mask_lo, m128 mask_hi, const u8 *buf,
                      const u8 *buf_end) {
    DEBUG_PRINTF("rshufti scan over %td bytes\n", buf_end - buf);
    svuint8_t sve_mask_lo = getSVEMaskFrom128(mask_lo);
    svuint8_t sve_mask_hi = getSVEMaskFrom128(mask_hi);
    const u8 *ptr = rshuftiSearch(sve_mask_lo, sve_mask_hi, buf, buf_end);
    return ptr ? ptr : buf - 1;
}

static really_inline
svbool_t doubleMatched(svuint8_t mask1_lo, svuint8_t mask1_hi,
                       svuint8_t mask2_lo, svuint8_t mask2_hi,
                       svuint8_t* inout_t1, const u8 *buf, const svbool_t pg) {
    svuint8_t vec = svld1_u8(pg, buf);

    svuint8_t chars_lo = svand_x(svptrue_b8(), vec, (uint8_t)0xf);
    svuint8_t chars_hi = svlsr_x(svptrue_b8(), vec, 4);

    svuint8_t c1_lo  = svtbl(mask1_lo, chars_lo);
    svuint8_t c1_hi  = svtbl(mask1_hi, chars_hi);
    svuint8_t new_t1 = svorr_z(svptrue_b8(), c1_lo, c1_hi);

    svuint8_t c2_lo  = svtbl(mask2_lo, chars_lo);
    svuint8_t c2_hi  = svtbl(mask2_hi, chars_hi);
    svuint8_t t2     = svorr_x(svptrue_b8(), c2_lo, c2_hi);

    // shift t1 left by one and feeds in the last element from the previous t1
    uint8_t last_elem = svlastb(svptrue_b8(), *inout_t1);
    svuint8_t merged_t1 = svinsr(new_t1, last_elem);
    svuint8_t t      = svorr_x(svptrue_b8(), merged_t1, t2);
    *inout_t1 = new_t1;

    return svnot_z(pg, svcmpeq(svptrue_b8(), t, (uint8_t)0xff));
}

static really_inline
const u8 *check_last_byte(svuint8_t mask2_lo, svuint8_t mask2_hi,
                    uint8_t last_elem, const u8 *buf_end) {
    uint8_t wild_lo = svorv(svptrue_b8(), mask2_lo);
    uint8_t wild_hi = svorv(svptrue_b8(), mask2_hi);
    uint8_t match_inverted = wild_lo | wild_hi | last_elem;
    // Also stop when the last byte is the first char of a double (last_elem): its
    // second char may be in the next stream chunk. This returns a resume point, not
    // a match, so it adds no false positives but fixes cross-chunk false negatives.
    int match = (match_inverted != 0xff) || (last_elem != 0xff);
    if(match) {
        return buf_end - 1;
    }
    return NULL;
}

static really_inline
const u8 *dshuftiOnce(svuint8_t mask1_lo, svuint8_t mask1_hi,
                      svuint8_t mask2_lo, svuint8_t mask2_hi,
                      svuint8_t *inout_t1, const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("start %p end %p\n", buf, buf_end);
    assert(buf < buf_end);
    DEBUG_PRINTF("l = %td\n", buf_end - buf);
    svbool_t pg = svwhilelt_b8_s64(0, buf_end - buf);
    svbool_t matched = doubleMatched(mask1_lo, mask1_hi, mask2_lo, mask2_hi,
                                     inout_t1, buf, pg);
    // doubleMatched return match position of the second char, but here we
    // return the position of the first char, hence the buffer offset
    return accelSearchCheckMatched(buf - 1, matched);
}

static really_inline
const u8 *dshuftiLoopBody(svuint8_t mask1_lo, svuint8_t mask1_hi,
                          svuint8_t mask2_lo, svuint8_t mask2_hi,
                          svuint8_t *inout_t1, const u8 *buf) {
    DEBUG_PRINTF("start %p end %p\n", buf, buf + svcntb());
    svbool_t matched = doubleMatched(mask1_lo, mask1_hi, mask2_lo, mask2_hi,
                                     inout_t1, buf, svptrue_b8());
    // doubleMatched return match position of the second char, but here we
    // return the position of the first char, hence the buffer offset
    return accelSearchCheckMatched(buf - 1, matched);
}

static really_inline
const u8 *dshuftiSearch(svuint8_t mask1_lo, svuint8_t mask1_hi,
                        svuint8_t mask2_lo, svuint8_t mask2_hi,
                        const u8 *buf, const u8 *buf_end) {
    assert(buf < buf_end);
    svuint8_t inout_t1 = svdup_u8(0xff);
    size_t len = buf_end - buf;
    if (len > svcntb()) {
        // peel off first part to align to the vector size
        const u8 *aligned_buf = ROUNDUP_PTR(buf, svcntb_pat(SV_POW2));
        assert(aligned_buf < buf_end);
        if (buf != aligned_buf) {
            const u8 *ptr = dshuftiLoopBody(mask1_lo, mask1_hi, mask2_lo,
                                            mask2_hi, &inout_t1, buf);
            if (ptr) return ptr;
            // The last match in inout won't line up with the next round as we
            // use an overlap. We need to set inout according to the last
            // unique-searched char.
            size_t offset = aligned_buf - buf;
            uint8_t last_unique_elem =
                svlastb(svwhilelt_b8(0UL, offset), inout_t1);
            inout_t1 = svdup_u8(last_unique_elem);
        }
        buf = aligned_buf;
        size_t loops = (buf_end - buf) / svcntb();
        DEBUG_PRINTF("loops %zu \n", loops);
        for (size_t i = 0; i < loops; i++, buf += svcntb()) {
            const u8 *ptr = dshuftiLoopBody(mask1_lo, mask1_hi, mask2_lo,
                                            mask2_hi, &inout_t1, buf);
            if (ptr) return ptr;
        }
        if (buf == buf_end) {
            uint8_t last_elem = svlastb(svptrue_b8(), inout_t1);
            return check_last_byte(mask2_lo, mask2_hi, last_elem, buf_end);
        }
    }
    DEBUG_PRINTF("buf %p buf_end %p \n", buf, buf_end);

    len = buf_end - buf;
    const u8 *ptr = dshuftiOnce(mask1_lo, mask1_hi,
                        mask2_lo, mask2_hi, &inout_t1, buf, buf_end);
    if (ptr) return ptr;
    uint8_t last_elem =
        svlastb(svwhilelt_b8(0UL, len), inout_t1);
    return check_last_byte(mask2_lo, mask2_hi, last_elem, buf_end);

}

const u8 *shuftiDoubleExec(m128 mask1_lo, m128 mask1_hi,
                           m128 mask2_lo, m128 mask2_hi,
                           const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("double shufti scan %td bytes\n", buf_end - buf);
    DEBUG_PRINTF("buf %p buf_end %p \n", buf, buf_end);
    svuint8_t sve_mask1_lo = getSVEMaskFrom128(mask1_lo);
    svuint8_t sve_mask1_hi = getSVEMaskFrom128(mask1_hi);
    svuint8_t sve_mask2_lo = getSVEMaskFrom128(mask2_lo);
    svuint8_t sve_mask2_hi = getSVEMaskFrom128(mask2_hi);
    const u8 *ptr = dshuftiSearch(sve_mask1_lo, sve_mask1_hi,
                                  sve_mask2_lo, sve_mask2_hi, buf, buf_end);
    return ptr ? ptr : buf_end;
}