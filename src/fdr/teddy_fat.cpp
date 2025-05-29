/*
 * Copyright (c) 2015-2020, Intel Corporation
 * Copyright (c) 2024, VectorCamp PC
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

/* fat teddy for AVX2 and AVX512VBMI */

#include "fdr_internal.h"
#include "flood_runtime.h"
#include "teddy.h"
#include "teddy_internal.h"
#include "teddy_runtime_common.h"
#include "util/arch.h"
#include "util/simd_utils.h"

#if defined(HAVE_AVX2)

#ifdef ARCH_64_BIT
static really_inline
hwlm_error_t conf_chunk_64(u64a chunk, u8 bucket, u8 offset,
                           CautionReason reason, const u8 *pt,
                           const u32* confBase,             
                           const struct FDR_Runtime_Args *a,           
                           hwlm_group_t *control,            
                           u32 *last_match) {    
    if (unlikely(chunk != ones_u64a)) {
        chunk = ~chunk;
        do_confWithBit_teddy(&chunk, bucket, offset, confBase, reason, a, pt,
                control, last_match);
        // adapted from CHECK_HWLM_TERMINATE_MATCHING    
        if (unlikely(*control == HWLM_TERMINATE_MATCHING)) {
            return HWLM_TERMINATED;
        }

    }
    return HWLM_SUCCESS;
}

#define CONF_FAT_CHUNK_64(chunk, bucket, off, reason, pt, confBase, a, control, last_match) \
 if(conf_chunk_64(chunk, bucket, off, reason, pt, confBase, a, control, last_match) == HWLM_TERMINATED)return HWLM_TERMINATED;
#else
static really_inline
hwlm_error_t conf_chunk_32(u32 chunk, u8 bucket, u8 offset,
                           CautionReason reason, const u8 *pt,
                           const u32* confBase,
                           const struct FDR_Runtime_Args *a,
                           hwlm_group_t *control,
                           u32 *last_match) {
    if (unlikely(chunk != ones_u32)) {
        chunk = ~chunk;
        do_confWithBit_teddy(&chunk, bucket, offset, confBase, reason, a, pt,
                control, last_match);
        // adapted from CHECK_HWLM_TERMINATE_MATCHING
        if (unlikely(*control == HWLM_TERMINATE_MATCHING)) {
            return HWLM_TERMINATED;
        }
    }
    return HWLM_SUCCESS;
}


#define CONF_FAT_CHUNK_32(chunk, bucket, off, reason, pt, confBase, a, control, last_match) \
 if(conf_chunk_32(chunk, bucket, off, reason, pt, confBase, a, control, last_match) == HWLM_TERMINATED)return HWLM_TERMINATED;

#endif


#if defined(HAVE_AVX512VBMI) // VBMI strong teddy

 // fat 512 teddy is only with vbmi

static really_inline
const m512 *getDupMaskBase(const struct Teddy *teddy, u8 numMask) {
    return (const m512 *)((const u8 *)teddy + ROUNDUP_CL(sizeof(struct Teddy))
                          + ROUNDUP_CL(2 * numMask * sizeof(m256)));
}


const u8 ALIGN_CL_DIRECTIVE p_mask_interleave[64] = {
    0, 32, 1, 33, 2, 34, 3, 35, 4, 36, 5, 37, 6, 38, 7, 39,
    8, 40, 9, 41, 10, 42, 11, 43, 12, 44, 13, 45, 14, 46, 15, 47,
    16, 48, 17, 49, 18, 50, 19, 51, 20, 52, 21, 53, 22, 54, 23, 55,
    24, 56, 25, 57, 26, 58, 27, 59, 28, 60, 29, 61, 30, 62, 31, 63
};

#ifdef ARCH_64_BIT
hwlm_error_t confirm_fat_teddy_64_512(m512 var, u8 bucket, u8 offset,
                                  CautionReason reason, const u8 *ptr,
                                  const struct FDR_Runtime_Args *a,
                                  const u32* confBase, hwlm_group_t *control,
                                  u32 *last_match) {
    if (unlikely(diff512(var, ones512()))) {
        m512 msk_interleave = load512(p_mask_interleave);
        m512 r = vpermb512(msk_interleave, var);
        m128 r0 = extract128from512(r, 0);
        m128 r1 = extract128from512(r, 1);
        m128 r2 = extract128from512(r, 2);
        m128 r3 = extract128from512(r, 3);
        u64a part1 = movq(r0);
        u64a part2 = extract64from128(r0, 1);
        u64a part3 = movq(r1);
        u64a part4 = extract64from128(r1, 1);
        u64a part5 = movq(r2);
        u64a part6 = extract64from128(r2, 1);
        u64a part7 = movq(r3);
        u64a part8 = extract64from128(r3, 1);
        CONF_FAT_CHUNK_64(part1, bucket, offset, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part2, bucket, offset + 4, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part3, bucket, offset + 8, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part4, bucket, offset + 12, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part5, bucket, offset + 16, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part6, bucket, offset + 20, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part7, bucket, offset + 24, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part8, bucket, offset + 28, reason, ptr, confBase, a, control, last_match);
    }
    return HWLM_SUCCESS;
}
#define confirm_fat_teddy_512_f confirm_fat_teddy_64_512
#else // 32-64

hwlm_error_t confirm_fat_teddy_32_512(m512 var, u8 bucket, u8 offset,
                                  CautionReason reason, const u8 *ptr,
                                  const struct FDR_Runtime_Args *a,
                                  const u32* confBase, hwlm_group_t *control,
                                  u32 *last_match) {
    if (unlikely(diff512(var, ones512()))) {
        m512 msk_interleave = load512(p_mask_interleave);
        m512 r = vpermb512(msk_interleave, var);
        m128 r0 = extract128from512(r, 0);
        m128 r1 = extract128from512(r, 1);
        m128 r2 = extract128from512(r, 2);
        m128 r3 = extract128from512(r, 3);
        u32 part1 = movd(r0);
        u32 part2 = extract32from128(r0, 1);
        u32 part3 = extract32from128(r0, 2);
        u32 part4 = extract32from128(r0, 3);
        u32 part5 = movd(r1);
        u32 part6 = extract32from128(r1, 1);
        u32 part7 = extract32from128(r1, 2);
        u32 part8 = extract32from128(r1, 3);
        u32 part9 = movd(r2);
        u32 part10 = extract32from128(r2, 1);
        u32 part11 = extract32from128(r2, 2);
        u32 part12 = extract32from128(r2, 3);
        u32 part13 = movd(r3);
        u32 part14 = extract32from128(r3, 1);
        u32 part15 = extract32from128(r3, 2);
        u32 part16 = extract32from128(r3, 3);
        CONF_FAT_CHUNK_32(part1, bucket, offset, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part2, bucket, offset + 2, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part3, bucket, offset + 4, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part4, bucket, offset + 6, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part5, bucket, offset + 8, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part6, bucket, offset + 10, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part7, bucket, offset + 12, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part8, bucket, offset + 14, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part9, bucket, offset + 16, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part10, bucket, offset + 18, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part11, bucket, offset + 20, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part12, bucket, offset + 22, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part13, bucket, offset + 24, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part14, bucket, offset + 26, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part15, bucket, offset + 28, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part16, bucket, offset + 30, reason, ptr, confBase, a, control, last_match);
    }
    return HWLM_SUCCESS;
}
#define confirm_fat_teddy_512_f confirm_fat_teddy_32_512
#endif // 32/64

#define CONFIRM_FAT_TEDDY_512(...) if(confirm_fat_teddy_512_f(__VA_ARGS__, a, confBase, &control, &last_match) == HWLM_TERMINATED)return HWLM_TERMINATED;

#define TEDDY_VBMI_SL1_MASK   0xfffffffffffffffeULL
#define TEDDY_VBMI_SL2_MASK   0xfffffffffffffffcULL
#define TEDDY_VBMI_SL3_MASK   0xfffffffffffffff8ULL

#define FAT_TEDDY_VBMI_SL1_MASK   0xfffffffefffffffeULL
#define FAT_TEDDY_VBMI_SL2_MASK   0xfffffffcfffffffcULL
#define FAT_TEDDY_VBMI_SL3_MASK   0xfffffff8fffffff8ULL

#define FAT_TEDDY_VBMI_SL1_POS    15
#define FAT_TEDDY_VBMI_SL2_POS    14
#define FAT_TEDDY_VBMI_SL3_POS    13

#define FAT_TEDDY_VBMI_CONF_MASK_HEAD   (0xffffffffULL >> n_sh)
#define FAT_TEDDY_VBMI_CONF_MASK_FULL   ((0xffffffffULL << n_sh) & 0xffffffffULL)
#define FAT_TEDDY_VBMI_CONF_MASK_VAR(n) (0xffffffffULL >> (32 - n) << overlap)
#define FAT_TEDDY_VBMI_LOAD_MASK_PATCH  (0xffffffffULL >> (32 - n_sh))

template<int NMSK>
static really_inline
m512 prep_conf_fat_teddy_512vbmi_templ(const m512 *lo_mask, const m512 *dup_mask,
                                       const m512 *sl_msk, const m512 val) {
    m512 lo = and512(val, *lo_mask);
    m512 hi = and512(rshift64_m512(val, 4), *lo_mask);
    m512 shuf_or_b0 = or512(pshufb_m512(dup_mask[0], lo),
                            pshufb_m512(dup_mask[1], hi));

    if constexpr (NMSK == 1) return shuf_or_b0;
    m512 shuf_or_b1 = or512(pshufb_m512(dup_mask[2], lo),
                            pshufb_m512(dup_mask[3], hi));
    m512 sl1 = maskz_vpermb512(FAT_TEDDY_VBMI_SL1_MASK, sl_msk[0], shuf_or_b1);
    if constexpr (NMSK == 2) return (or512(sl1, shuf_or_b0));
    m512 shuf_or_b2 = or512(pshufb_m512(dup_mask[4], lo),
                            pshufb_m512(dup_mask[5], hi));
    m512 sl2 = maskz_vpermb512(FAT_TEDDY_VBMI_SL2_MASK, sl_msk[1], shuf_or_b2);
    if constexpr (NMSK == 3) return (or512(sl2, or512(sl1, shuf_or_b0)));
    m512 shuf_or_b3 = or512(pshufb_m512(dup_mask[6], lo),
                            pshufb_m512(dup_mask[7], hi));
    m512 sl3 = maskz_vpermb512(FAT_TEDDY_VBMI_SL3_MASK, sl_msk[2], shuf_or_b3);
    return (or512(sl3, or512(sl2, or512(sl1, shuf_or_b0))));
}


#define TEDDY_VBMI_SL1_POS    15
#define TEDDY_VBMI_SL2_POS    14
#define TEDDY_VBMI_SL3_POS    13

#define TEDDY_VBMI_CONF_MASK_HEAD   (0xffffffffffffffffULL >> n_sh)
#define TEDDY_VBMI_CONF_MASK_FULL   (0xffffffffffffffffULL << n_sh)
#define TEDDY_VBMI_CONF_MASK_VAR(n) (0xffffffffffffffffULL >> (64 - n) << overlap)
#define TEDDY_VBMI_LOAD_MASK_PATCH  (0xffffffffffffffffULL >> (64 - n_sh))

template<int NMSK>
hwlm_error_t fdr_exec_fat_teddy_512vbmi_templ(const struct FDR *fdr,
                                              const struct FDR_Runtime_Args *a,
                                              hwlm_group_t control) {
    const u8 *buf_end = a->buf + a->len;
    const u8 *ptr = a->buf + a->start_offset;
    u32 floodBackoff = FLOOD_BACKOFF_START;
    const u8 *tryFloodDetect = a->firstFloodDetect;
    u32 last_match = ones_u32;
    const struct Teddy *teddy = (const struct Teddy *)fdr;
    const size_t iterBytes = 32;
    u32 n_sh = NMSK - 1;
    const size_t loopBytes = 32 - n_sh;
    DEBUG_PRINTF("params: buf %p len %zu start_offset %zu\n",
                 a->buf, a->len, a->start_offset);

    const m512 *dup_mask = getDupMaskBase(teddy, NMSK);
    m512 lo_mask = set1_64x8(0xf);
    m512 sl_msk[NMSK - 1];
    if constexpr (NMSK > 1){
    sl_msk[0] = loadu512(p_sh_mask_arr + FAT_TEDDY_VBMI_SL1_POS);
    }
    if constexpr (NMSK > 2){
    sl_msk[1] = loadu512(p_sh_mask_arr + FAT_TEDDY_VBMI_SL2_POS);
    }
    if constexpr (NMSK > 3){
    sl_msk[2] = loadu512(p_sh_mask_arr + FAT_TEDDY_VBMI_SL3_POS);
    }

    const u32 *confBase = getConfBase(teddy);

    u64a k = FAT_TEDDY_VBMI_CONF_MASK_FULL;
    m512 p_mask = set_mask_m512(~((k << 32) | k));
    u32 overlap = 0;
    u64a patch = 0;
    if (likely(ptr + loopBytes <= buf_end)) {
        u64a k0 = FAT_TEDDY_VBMI_CONF_MASK_HEAD;
        m512 p_mask0 = set_mask_m512(~((k0 << 32) | k0));
        m512 r_0 = prep_conf_fat_teddy_512vbmi_templ<NMSK>(&lo_mask, dup_mask, sl_msk, set2x256(loadu_maskz_m256(k0, ptr)));

        r_0 = or512(r_0, p_mask0);
        CONFIRM_FAT_TEDDY_512(r_0, 16, 0, VECTORING, ptr);
        ptr += loopBytes;
        overlap = n_sh;
        patch = FAT_TEDDY_VBMI_LOAD_MASK_PATCH;
    }

    for (; ptr + loopBytes <= buf_end; ptr += loopBytes) {
        CHECK_FLOOD;
        m512 r_0 = prep_conf_fat_teddy_512vbmi_templ<NMSK>(&lo_mask, dup_mask, sl_msk, set2x256(loadu256(ptr - n_sh)));
        r_0 = or512(r_0, p_mask);
        CONFIRM_FAT_TEDDY_512(r_0, 16, 0, NOT_CAUTIOUS, ptr - n_sh);
    }

    assert(ptr + loopBytes > buf_end);
    if (ptr < buf_end) {
        u32 left = (u32)(buf_end - ptr);
        u64a k1 = FAT_TEDDY_VBMI_CONF_MASK_VAR(left);
        m512 p_mask1 = set_mask_m512(~((k1 << 32) | k1));
        m512 val_0 = set2x256(loadu_maskz_m256(k1 | patch, ptr - overlap));
        m512 r_0 = prep_conf_fat_teddy_512vbmi_templ<NMSK>(&lo_mask, dup_mask, sl_msk, val_0);

        r_0 = or512(r_0, p_mask1);
        CONFIRM_FAT_TEDDY_512(r_0, 16, 0, VECTORING, ptr - overlap);
    }

    return HWLM_SUCCESS;
}

#define FDR_EXEC_FAT_TEDDY_FN fdr_exec_fat_teddy_512vbmi_templ


#elif defined(HAVE_AVX2) // not HAVE_AVX512 but HAVE_AVX2 reinforced teddy


#ifdef ARCH_64_BIT
extern "C" {
hwlm_error_t confirm_fat_teddy_64_256(m256 var, u8 bucket, u8 offset,
                                      CautionReason reason, const u8 *ptr,
                                      const struct FDR_Runtime_Args *a,
                                      const u32* confBase, hwlm_group_t *control,
                                      u32 *last_match) {
    if (unlikely(diff256(var, ones256()))) {
        m256 swap = swap128in256(var);
        m256 r = interleave256lo(var, swap);
        u64a part1 = extractlow64from256(r);
        u64a part2 = extract64from256(r, 1);
        r = interleave256hi(var, swap);
        u64a part3 = extractlow64from256(r);
        u64a part4 = extract64from256(r, 1);
        CONF_FAT_CHUNK_64(part1, bucket, offset, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part2, bucket, offset + 4, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part3, bucket, offset + 8, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_64(part4, bucket, offset + 12, reason, ptr, confBase, a, control, last_match);
    }
    return HWLM_SUCCESS;
}
} // extern C

#define confirm_fat_teddy_256_f confirm_fat_teddy_64_256

#else
extern "C" {
hwlm_error_t confirm_fat_teddy_32_256(m256 var, u8 bucket, u8 offset,
                                      CautionReason reason, const u8 *ptr,
                                      const struct FDR_Runtime_Args *a,
                                      const u32* confBase, hwlm_group_t *control,
                                      u32 *last_match) {
    if (unlikely(diff256(var, ones256()))) {
        m256 swap = swap128in256(var);
        m256 r = interleave256lo(var, swap);
        u32 part1 = extractlow32from256(r);
        u32 part2 = extract32from256(r, 1);
        u32 part3 = extract32from256(r, 2);
        u32 part4 = extract32from256(r, 3);
        r = interleave256hi(var, swap);
        u32 part5 = extractlow32from256(r);
        u32 part6 = extract32from256(r, 1);
        u32 part7 = extract32from256(r, 2);
        u32 part8 = extract32from256(r, 3);
        CONF_FAT_CHUNK_32(part1, bucket, offset, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part2, bucket, offset + 2, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part3, bucket, offset + 4, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part4, bucket, offset + 6, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part5, bucket, offset + 8, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part6, bucket, offset + 10, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part7, bucket, offset + 12, reason, ptr, confBase, a, control, last_match);
        CONF_FAT_CHUNK_32(part8, bucket, offset + 14, reason, ptr, confBase, a, control, last_match);
    }
    return HWLM_SUCCESS;
}

} // extern C

#define confirm_fat_teddy_256_f confirm_fat_teddy_32_256

#endif

#define CONFIRM_FAT_TEDDY_256(...) if(confirm_fat_teddy_256_f(__VA_ARGS__, a, confBase, &control, &last_match) == HWLM_TERMINATED)return HWLM_TERMINATED;

static really_inline
const m256 *getMaskBase_fat(const struct Teddy *teddy) {
    return (const m256 *)((const u8 *)teddy + ROUNDUP_CL(sizeof(struct Teddy)));
}


static really_inline
m256 vectoredLoad2x128(m256 *p_mask, const u8 *ptr, const size_t start_offset,
                       const u8 *lo, const u8 *hi,
                       const u8 *buf_history, size_t len_history,
                       const u32 nMasks) {
    m128 p_mask128;
    m256 ret = set1_2x128(vectoredLoad128(&p_mask128, ptr, start_offset, lo, hi,
                                          buf_history, len_history, nMasks));
    *p_mask = set1_2x128(p_mask128);
    return ret;
}

template<int NMSK>
static really_inline
m256 prep_conf_fat_teddy_256_templ(const m256 *maskBase, m256 val,
                                   m256* old_1, m256* old_2, m256* old_3){
    m256 mask = set1_32x8(0xf);
    m256 lo = and256(val, mask);
    m256 hi = and256(rshift64_m256(val, 4), mask);
    m256 r = or256(pshufb_m256(maskBase[0 * 2], lo),
                     pshufb_m256(maskBase[0 * 2 + 1], hi));
    if constexpr (NMSK == 1) return r;
    m256 res_1 = or256(pshufb_m256(maskBase[(NMSK-1) * 2], lo),
                       pshufb_m256(maskBase[(NMSK-1) * 2 + 1], hi));
    m256 res_shifted_1 = vpalignr(res_1, *old_1, 16 - (NMSK-1));
    *old_1 = res_1;
    r = or256(r, res_shifted_1);
    if constexpr (NMSK == 2) return r;
    m256 res_2 = or256(pshufb_m256(maskBase[(NMSK-1) * 2], lo),
                       pshufb_m256(maskBase[(NMSK-1) * 2 + 1], hi));
    m256 res_shifted_2 = vpalignr(res_2, *old_2, 16 - (NMSK-1));
    *old_2 = res_2;
    r = or256(r, res_shifted_2);
    if constexpr (NMSK == 3) return r;
    m256 res_3 = or256(pshufb_m256(maskBase[(NMSK-1) * 2], lo),
                       pshufb_m256(maskBase[(NMSK-1) * 2 + 1], hi));
    m256 res_shifted_3 = vpalignr(res_3, *old_3, 16 - (NMSK-1));
    *old_3 = res_3;
    return or256(r, res_shifted_3);
}

template<int NMSK>
hwlm_error_t fdr_exec_fat_teddy_256_templ(const struct FDR *fdr,
                                          const struct FDR_Runtime_Args *a,
                                          hwlm_group_t control) {
    const u8 *buf_end = a->buf + a->len;
    const u8 *ptr = a->buf + a->start_offset;
    u32 floodBackoff = FLOOD_BACKOFF_START;
    const u8 *tryFloodDetect = a->firstFloodDetect;
    u32 last_match = ones_u32;
    const struct Teddy *teddy = (const struct Teddy *)fdr;
    const size_t iterBytes = 32;
    DEBUG_PRINTF("params: buf %p len %zu start_offset %zu\n",
                 a->buf, a->len, a->start_offset);

    const m256 *maskBase = getMaskBase_fat(teddy);
    const u32 *confBase = getConfBase(teddy);

    m256 res_old_1 = zeroes256();
    m256 res_old_2 = zeroes256();
    m256 res_old_3 = zeroes256();
    const u8 *mainStart = ROUNDUP_PTR(ptr, 16);
    DEBUG_PRINTF("derive: ptr: %p mainstart %p\n", ptr, mainStart);
    if (ptr < mainStart) {
        ptr = mainStart - 16;
        m256 p_mask;
        m256 val_0 = vectoredLoad2x128(&p_mask, ptr, a->start_offset,
                                       a->buf, buf_end,
                                       a->buf_history, a->len_history,
                                       NMSK);
        m256 r_0 = prep_conf_fat_teddy_256_templ<NMSK>(maskBase, val_0, &res_old_1, &res_old_2, &res_old_3);
        r_0 = or256(r_0, p_mask);
        CONFIRM_FAT_TEDDY_256(r_0, 16, 0, VECTORING, ptr);
        ptr += 16;
    }

    if (ptr + 16 <= buf_end) {
        m256 r_0 = prep_conf_fat_teddy_256_templ<NMSK>(maskBase, load2x128(ptr), &res_old_1, &res_old_2, &res_old_3);
        CONFIRM_FAT_TEDDY_256(r_0, 16, 0, VECTORING, ptr);
        ptr += 16;
    }

    for ( ; ptr + iterBytes <= buf_end; ptr += iterBytes) {
        __builtin_prefetch(ptr + (iterBytes * 4));
        CHECK_FLOOD;
        m256 r_0 = prep_conf_fat_teddy_256_templ<NMSK>(maskBase, load2x128(ptr), &res_old_1, &res_old_2, &res_old_3);
        CONFIRM_FAT_TEDDY_256(r_0, 16, 0, NOT_CAUTIOUS, ptr);
        m256 r_1 = prep_conf_fat_teddy_256_templ<NMSK>(maskBase, load2x128(ptr + 16), &res_old_1, &res_old_2, &res_old_3);
        CONFIRM_FAT_TEDDY_256(r_1, 16, 16, NOT_CAUTIOUS, ptr);
    }

    if (ptr + 16 <= buf_end) {
        m256 r_0 = prep_conf_fat_teddy_256_templ<NMSK>(maskBase, load2x128(ptr), &res_old_1, &res_old_2, &res_old_3);
        CONFIRM_FAT_TEDDY_256(r_0, 16, 0, NOT_CAUTIOUS, ptr);
        ptr += 16;
    }

    assert(ptr + 16 > buf_end);
    if (ptr < buf_end) {
        m256 p_mask;
        m256 val_0 = vectoredLoad2x128(&p_mask, ptr, 0, ptr, buf_end,
                                       a->buf_history, a->len_history,
                                       NMSK);
        m256 r_0 = prep_conf_fat_teddy_256_templ<NMSK>(maskBase, val_0, &res_old_1, &res_old_2, &res_old_3);
        r_0 = or256(r_0, p_mask);
        CONFIRM_FAT_TEDDY_256(r_0, 16, 0, VECTORING, ptr);
    }
    return HWLM_SUCCESS;
}

// this check is because it is possible to build with both AVX512VBMI and AVX2 defined,
// to replicate the behaviour of the original flow of control we give preference
// to the former. If we're building for both then this will be compiled multiple times
// with the desired variant defined by itself.
#ifndef FDR_EXEC_FAT_TEDDY_FN
#define FDR_EXEC_FAT_TEDDY_FN fdr_exec_fat_teddy_256_templ
#endif

#endif // HAVE_AVX2 for fat teddy

/* we only have fat teddy in these two modes */
// #if (defined(HAVE_AVX2) || defined(HAVE_AVX512VBMI)) && defined(FDR_EXEC_FAT_TEDDY_FN)
// #if defined(FDR_EXEC_FAT_TEDDY_FN)

extern "C" {
hwlm_error_t fdr_exec_fat_teddy_msks1(const struct FDR *fdr,
                                      const struct FDR_Runtime_Args *a,
                                      hwlm_group_t control) {
    return FDR_EXEC_FAT_TEDDY_FN<1>(fdr, a, control);
}

hwlm_error_t fdr_exec_fat_teddy_msks1_pck(const struct FDR *fdr,
                                          const struct FDR_Runtime_Args *a,
                                          hwlm_group_t control) {
    return FDR_EXEC_FAT_TEDDY_FN<1>(fdr, a, control);
}

hwlm_error_t fdr_exec_fat_teddy_msks2(const struct FDR *fdr,
                                      const struct FDR_Runtime_Args *a,
                                      hwlm_group_t control) {
    return FDR_EXEC_FAT_TEDDY_FN<2>(fdr, a, control);
}

hwlm_error_t fdr_exec_fat_teddy_msks2_pck(const struct FDR *fdr,
                                          const struct FDR_Runtime_Args *a,
                                          hwlm_group_t control) {
    return FDR_EXEC_FAT_TEDDY_FN<2>(fdr, a, control);
}

hwlm_error_t fdr_exec_fat_teddy_msks3(const struct FDR *fdr,
                                      const struct FDR_Runtime_Args *a,
                                      hwlm_group_t control) {
    return FDR_EXEC_FAT_TEDDY_FN<3>(fdr, a, control);
}

hwlm_error_t fdr_exec_fat_teddy_msks3_pck(const struct FDR *fdr,
                                          const struct FDR_Runtime_Args *a,
                                          hwlm_group_t control) {
    return FDR_EXEC_FAT_TEDDY_FN<3>(fdr, a, control);
}

hwlm_error_t fdr_exec_fat_teddy_msks4(const struct FDR *fdr,
                                      const struct FDR_Runtime_Args *a,
                                      hwlm_group_t control) {
    return FDR_EXEC_FAT_TEDDY_FN<4>(fdr, a, control);
}

hwlm_error_t fdr_exec_fat_teddy_msks4_pck(const struct FDR *fdr,
                                          const struct FDR_Runtime_Args *a,
                                          hwlm_group_t control) {
    return FDR_EXEC_FAT_TEDDY_FN<4>(fdr, a, control);
}

} // extern c

#endif // HAVE_AVX2 from the beginning

