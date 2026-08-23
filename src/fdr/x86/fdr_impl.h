/*
 * Copyright (c) 2015-2017, Intel Corporation
 * Copyright (c) 2020-2025, VectorCamp PC
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

#ifndef FDR_IMPL_X86_H
#define FDR_IMPL_X86_H

static really_inline
void get_conf_stride(const u8 *itPtr, UNUSED const u8 *start_ptr,
                       UNUSED const u8 *end_ptr, u32 domain_mask, u8 stride,
                       const u64a *ft, u64a *conf0, u64a *conf8, m128 *s) {
    assert(itPtr >= start_ptr && itPtr + ITER_BYTES <= end_ptr);

    // get_conf_stride_4
    u64a it_hi = *(const u64a *)itPtr;
    u64a it_lo = *(const u64a *)(itPtr + 8);
    u64a reach0  = domain_mask & it_hi;
    u64a reach4  = domain_mask & (it_hi >> 32);
    u64a reach8  = domain_mask & it_lo;
    u64a reach12 = domain_mask & (it_lo >> 32);

    m128 st0 = load_m128_from_u64a(ft + reach0);
    m128 st4 = load_m128_from_u64a(ft + reach4);
    m128 st8 = load_m128_from_u64a(ft + reach8);
    m128 st12 = load_m128_from_u64a(ft + reach12);

    st4 = lshiftbyte_m128(st4, 4);
    st12 = lshiftbyte_m128(st12, 4);

    *s = or128(*s, st0);
    *s = or128(*s, st4);

    if (stride == 4) {
        *conf0 = movq(*s);
        *s = rshiftbyte_m128(*s, 8);
        *conf0 ^= ~0ULL;

        *s = or128(*s, st8);
        *s = or128(*s, st12);
        *conf8 = movq(*s);
        *s = rshiftbyte_m128(*s, 8);
        *conf8 ^= ~0ULL;
        return;
    }

    // get_conf_stride_2
    u64a reach2  = domain_mask & (it_hi >> 16);
    u64a reach6  = domain_mask & (it_hi >> 48);
    u64a reach10 = domain_mask & (it_lo >> 16);
    u64a reach14 = domain_mask & (it_lo >> 48);
    
    m128 st2 = load_m128_from_u64a(ft + reach2);
    m128 st6 = load_m128_from_u64a(ft + reach6);
    m128 st10 = load_m128_from_u64a(ft + reach10);
    m128 st14 = load_m128_from_u64a(ft + reach14);

    st2  = lshiftbyte_m128(st2, 2);
    st6  = lshiftbyte_m128(st6, 6);
    st10 = lshiftbyte_m128(st10, 2);
    st14 = lshiftbyte_m128(st14, 6);

    *s = or128(*s, st2);
    *s = or128(*s, st6);

    if (stride == 2) {
        *conf0 = movq(*s);
        *s = rshiftbyte_m128(*s, 8);
        *conf0 ^= ~0ULL;

        *s = or128(*s, st8);
        *s = or128(*s, st10);
        *s = or128(*s, st12);
        *s = or128(*s, st14);

        *conf8 = movq(*s);
        *s = rshiftbyte_m128(*s, 8);
        *conf8 ^= ~0ULL;
        return;
    }

    // get_conf_stride_1
    u64a reach1  = domain_mask & (it_hi >> 8);
    u64a reach3  = domain_mask & (it_hi >> 24);
    u64a reach5  = domain_mask & (it_hi >> 40);
    u64a reach7  = domain_mask & ((it_hi >> 56) | (it_lo << 8));
    u64a reach9  = domain_mask & (it_lo >> 8);
    u64a reach11 = domain_mask & (it_lo >> 24);
    u64a reach13 = domain_mask & (it_lo >> 40);
    u64a reach15 = domain_mask & unaligned_load_u32(itPtr + 15);

    m128 st1 = load_m128_from_u64a(ft + reach1);
    m128 st3 = load_m128_from_u64a(ft + reach3);
    m128 st5 = load_m128_from_u64a(ft + reach5);
    m128 st7 = load_m128_from_u64a(ft + reach7);
    m128 st9 = load_m128_from_u64a(ft + reach9);
    m128 st11 = load_m128_from_u64a(ft + reach11);
    m128 st13 = load_m128_from_u64a(ft + reach13);
    m128 st15 = load_m128_from_u64a(ft + reach15);

    st1 = lshiftbyte_m128(st1, 1);
    st3 = lshiftbyte_m128(st3, 3);
    st5 = lshiftbyte_m128(st5, 5);
    st7 = lshiftbyte_m128(st7, 7);
    st9 = lshiftbyte_m128(st9, 1);
    st11 = lshiftbyte_m128(st11, 3);
    st13 = lshiftbyte_m128(st13, 5);
    st15 = lshiftbyte_m128(st15, 7);

    st0 = or128(st0, st1);
    st2 = or128(st2, st3);
    st4 = or128(st4, st5);
    st6 = or128(st6, st7);
    st0 = or128(st0, st2);
    st4 = or128(st4, st6);
    st0 = or128(st0, st4);

    st8 = or128(st8, st9);
    st10 = or128(st10, st11);
    st12 = or128(st12, st13);
    st14 = or128(st14, st15);
    st8 = or128(st8, st10);
    st12 = or128(st12, st14);
    st8 = or128(st8, st12);

    m128 st = or128(*s, st0);
    *conf0 = movq(st) ^ ~0ULL;
    st = rshiftbyte_m128(st, 8);
    st = or128(st, st8);

    *conf8 = movq(st) ^ ~0ULL;
    *s = rshiftbyte_m128(st, 8);
}

static really_inline
void do_confirm_fdr(u64a *conf, u8 offset, hwlmcb_rv_t *control,
                    const u32 *confBase, const struct FDR_Runtime_Args *a,
                    const u8 *ptr, u32 *last_match_id, const struct zone *z) {
    const u8 bucket = 8;

    if (likely(!*conf)) {
        return;
    }

    /* ptr is currently referring to a location in the zone's buffer, we also
     * need a pointer in the original, main buffer for the final string compare.
     */
    const u8 *ptr_main = (const u8 *)((uintptr_t)ptr + z->zone_pointer_adjust); //NOLINT (performance-no-int-to-ptr)

    const u8 *confLoc = ptr;

    do  {
        u32 bit = findAndClearLSB_64(conf);
        u32 byte = bit / bucket + offset;
        u32 bitRem = bit % bucket;
        u32 idx = bitRem;
        u32 cf = confBase[idx];
        if (!cf) {
            continue;
        }
        const struct FDRConfirm *fdrc = (const struct FDRConfirm *)
                                        ((const u8 *)confBase + cf);
        if (!(fdrc->groups & *control)) {
            continue;
        }
        u64a confVal = unaligned_load_u64a(confLoc + byte - sizeof(u64a) + 1);
        confWithBit(fdrc, a, ptr_main - a->buf + byte, control,
                    last_match_id, confVal, conf, bit);
    } while (unlikely(!!*conf));
}

#endif // FDR_IMPL_X86_H