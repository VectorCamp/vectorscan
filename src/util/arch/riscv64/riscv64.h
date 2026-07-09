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

/** \file
 * \brief Per-platform architecture definitions for RISC-V 64-bit
 */

#ifndef UTIL_ARCH_RISCV64_H_
#define UTIL_ARCH_RISCV64_H_

#if defined(__riscv) && __riscv_xlen == 64
#define HAVE_RISCV64
#endif

#if defined(__riscv_v_intrinsic) || defined(__riscv_vector)
#define HAVE_RVV
#endif

#if defined(HAVE_RVV)
#define HAVE_SIMD_128_BITS
#define CHUNKSIZE 128
#define VECTORSIZE 16
#endif

#if defined(HAVE_RVV) && defined(__riscv_v_elen) && (__riscv_v_elen >= 256)
#define HAVE_SIMD_256_BITS
#endif

/* RISC-V Bitmanip extension (Zbb) provides ctz/clz/popcount */
#if defined(__riscv_zbb)
#define HAVE_ZBB
#endif

/* RISC-V Carryless Multiply extension (Zbc) */
#if defined(__riscv_zbc)
#define HAVE_ZBC
#endif

/* RISC-V Zvbb - Vector Bitmanip */
#if defined(__riscv_zvbb)
#define HAVE_ZVBB
#endif

/* RISC-V Zvbc - Vector Carryless Multiply */
#if defined(__riscv_zvbc)
#define HAVE_ZVBC
#endif

/* RISC-V Zicsr extension */
#if defined(__riscv_zicsr)
#define HAVE_ZICSR
#endif

#endif // UTIL_ARCH_RISCV64_H_
