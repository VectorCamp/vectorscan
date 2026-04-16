/*
 * Copyright (c) 2016-2017, Intel Corporation
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

#include "config.h"
#include "hs_common.h"
#include "ue2common.h"
#if !defined(VS_SIMDE_BACKEND)
#if defined(ARCH_IA32) || defined(ARCH_X86_64)
#include "util/arch/x86/cpuid_inline.h"
#elif defined(ARCH_AARCH64)
#include "util/arch/arm/cpuid_inline.h"
#endif
#endif

#include <stdio.h>

HS_PUBLIC_API
hs_error_t HS_CDECL hs_valid_platform(void) {
    /* Vectorscan requires SSE4.2, anything else is a bonus */
#if defined(FAT_RUNTIME)
#if (defined(ARCH_IA32) || defined(ARCH_X86_64))
#if defined(VS_SIMDE_BACKEND)
	// Now that we have SIMDe fallback we can always return success.
    return HS_SUCCESS;
#else
	printf("checking SSE42 support\n");
	return check_sse42()? HS_SUCCESS: HS_ARCH_ERROR;
#endif
#elif (defined(ARCH_ARM32) || defined(ARCH_AARCH64))
    // cppcheck-suppress knownConditionTrueFalse
    return check_neon()? HS_SUCCESS: HS_ARCH_ERROR;
#endif
#else
#if !defined(VS_SIMDE_BACKEND) && (defined(ARCH_IA32) || defined(ARCH_X86_64))
    return check_sse42()? HS_SUCCESS: HS_ARCH_ERROR;
#elif !defined(VS_SIMDE_BACKEND) && (defined(ARCH_ARM32) || defined(ARCH_AARCH64))
    // cppcheck-suppress knownConditionTrueFalse
    return check_neon()? HS_SUCCESS: HS_ARCH_ERROR;
#elif defined(ARCH_PPC64EL) || defined(VS_SIMDE_BACKEND)
    return HS_SUCCESS;
#endif
#endif
    return HS_SUCCESS;
}
