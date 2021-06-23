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
 * \brief Vermicelli: single-byte and double-byte acceleration.
 */

#ifndef VERMICELLI_H
#define VERMICELLI_H

#include "util/bitutils.h"
#include "util/simd_utils.h"
#include "util/unaligned.h"

#if !defined(HAVE_AVX512)
#include "vermicelli_common.h"
#endif

#ifdef HAVE_SVE2
#include "vermicelli_sve.h"
#else
#include "vermicelli_sse.h"
#endif

static really_inline
const u8 *vermicelliDoubleMaskedExec(char c1, char c2, char m1, char m2,
                                     const u8 *buf, const u8 *buf_end) {
    DEBUG_PRINTF("double verm scan (\\x%02hhx&\\x%02hhx)(\\x%02hhx&\\x%02hhx) "
                 "over %zu bytes\n", c1, m1, c2, m2, (size_t)(buf_end - buf));
    assert(buf < buf_end);

    VERM_TYPE chars1 = VERM_SET_FN(c1);
    VERM_TYPE chars2 = VERM_SET_FN(c2);
    VERM_TYPE mask1 = VERM_SET_FN(m1);
    VERM_TYPE mask2 = VERM_SET_FN(m2);

#ifdef HAVE_AVX512
    if (buf_end - buf <= VERM_BOUNDARY) {
        const u8 *ptr = dvermMiniMasked(chars1, chars2, mask1, mask2, buf,
                                        buf_end);
        if (ptr) {
            return ptr;
        }

        /* check for partial match at end */
        if ((buf_end[-1] & m1) == (u8)c1) {
            DEBUG_PRINTF("partial!!!\n");
            return buf_end - 1;
        }

        return buf_end;
    }
#endif

    assert((buf_end - buf) >= VERM_BOUNDARY);
    uintptr_t min = (uintptr_t)buf % VERM_BOUNDARY;
    if (min) {
        // Input isn't aligned, so we need to run one iteration with an
        // unaligned load, then skip buf forward to the next aligned address.
        // There's some small overlap here, but we don't mind scanning it twice
        // if we can do it quickly, do we?
        const u8 *p = dvermPreconditionMasked(chars1, chars2, mask1, mask2, buf);
        if (p) {
            return p;
        }

        buf += VERM_BOUNDARY - min;
        assert(buf < buf_end);
    }

    // Aligned loops from here on in
    const u8 *ptr = dvermSearchAlignedMasked(chars1, chars2, mask1, mask2, c1,
                                             c2, m1, m2, buf, buf_end);
    if (ptr) {
        return ptr;
    }

    // Tidy up the mess at the end
    ptr = dvermPreconditionMasked(chars1, chars2, mask1, mask2,
                                  buf_end - VERM_BOUNDARY);

    if (ptr) {
        return ptr;
    }

    /* check for partial match at end */
    if ((buf_end[-1] & m1) == (u8)c1) {
        DEBUG_PRINTF("partial!!!\n");
        return buf_end - 1;
    }

    return buf_end;
}

#endif /* VERMICELLI_H */
