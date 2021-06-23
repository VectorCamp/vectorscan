/*
 * Copyright (c) 2015-2020, Intel Corporation
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
 * \brief Vermicelli: Implementation shared between architectures.
 *
 * (users should include vermicelli.h instead of this)
 */

#define VERM_BOUNDARY 16
#define VERM_TYPE m128
#define VERM_SET_FN set1_16x8

// returns NULL if not found
static really_inline
const u8 *dvermPreconditionMasked(m128 chars1, m128 chars2,
                                  m128 mask1, m128 mask2, const u8 *buf) {
    m128 data = loadu128(buf); // unaligned
    m128 v1 = eq128(chars1, and128(data, mask1));
    m128 v2 = eq128(chars2, and128(data, mask2));
    u32 z = movemask128(and128(v1, rshiftbyte_m128(v2, 1)));

    /* no fixup of the boundary required - the aligned run will pick it up */
    if (unlikely(z)) {
        u32 pos = ctz32(z);
        return buf + pos;
    }
    return NULL;
}

static really_inline
const u8 *dvermSearchAlignedMasked(m128 chars1, m128 chars2,
                                   m128 mask1, m128 mask2, u8 c1, u8 c2, u8 m1,
                                   u8 m2, const u8 *buf, const u8 *buf_end) {
    assert((size_t)buf % 16 == 0);

    for (; buf + 16 < buf_end; buf += 16) {
        m128 data = load128(buf);
        m128 v1 = eq128(chars1, and128(data, mask1));
        m128 v2 = eq128(chars2, and128(data, mask2));
        u32 z = movemask128(and128(v1, rshiftbyte_m128(v2, 1)));

        if ((buf[15] & m1) == c1 && (buf[16] & m2) == c2) {
            z |= (1 << 15);
        }
        if (unlikely(z)) {
            u32 pos = ctz32(z);
            return buf + pos;
        }
    }

    return NULL;
}