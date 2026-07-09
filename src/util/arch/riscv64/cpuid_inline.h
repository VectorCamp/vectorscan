/*
 * Copyright (c) 2017-2020, Intel Corporation
 * Copyright (c) 2020-2026, VectorCamp PC
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

#ifndef CPUID_INLINE_RISCV64_H_
#define CPUID_INLINE_RISCV64_H_

#include "ue2common.h"
#include "util/arch/common/cpuid_flags.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* RISC-V does not have CPUID; feature detection is done at compile time
 * via predefined macros (__riscv_v, __riscv_zbb, etc.) or at runtime
 * via HWCAP on Linux (getauxval). */

/* Runtime RVV detection via HWCAP on Linux */
#if defined(__linux__) && defined(__riscv)
#include <sys/auxv.h>
#define HWCAP_RVV (1 << 12) /* COMPAT_HWCAP_ISA_V */

static inline
int check_rvv(void) {
    unsigned long hwcap = getauxval(AT_HWCAP);
    return !!(hwcap & HWCAP_RVV);
}
#else
static inline
int check_rvv(void) {
#if defined(__riscv_v_intrinsic) || defined(__riscv_vector)
    return 1;
#else
    return 0;
#endif
}
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CPUID_INLINE_RISCV64_H_ */
