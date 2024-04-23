/*
 * Copyright (c) 2020, 2021, VectorCamp PC
 * Copyright (c) 2024, Arm Limited
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

#include "hwlm/hwlm_literal.h"
#include "hwlm/noodle_build.h"
#include "hwlm/noodle_engine.h"
#include "hwlm/noodle_internal.h"
#include "nfa/shufti.h"
#include "nfa/shufticompile.h"
#include "nfa/truffle.h"
#include "nfa/trufflecompile.h"
#include "nfa/vermicelli.hpp"
#include "scratch.h"
#include "util/bytecode_ptr.h"

class MicroBenchmark {
public:
    char const *label;
    size_t size;

    // Shufti/Truffle
    union {
        m256 truffle_mask;
        struct {
#if (SIMDE_ENDIAN_ORDER == SIMDE_ENDIAN_LITTLE)
            m128 truffle_mask_lo;
            m128 truffle_mask_hi;
#else
            m128 truffle_mask_hi;
            m128 truffle_mask_lo;
#endif
        };
    };
    ue2::CharReach chars;
    std::vector<u8> buf;

    // Noodle
    struct hs_scratch scratch;
    ue2::bytecode_ptr<noodTable> nt;

    MicroBenchmark(char const *label_, size_t size_)
        : label(label_), size(size_), buf(size_){};
};
