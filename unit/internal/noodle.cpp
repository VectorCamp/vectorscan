/*
 * Copyright (c) 2015-2017, Intel Corporation
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

#include "ue2common.h"
#include "hwlm/noodle_build.h"
#include "hwlm/noodle_engine.h"
#include "hwlm/hwlm.h"
#include "hwlm/hwlm_literal.h"
#include "scratch.h"
#include "util/alloc.h"
#include "util/ue2string.h"

#include <cstring>
#include <vector>
#include "gtest/gtest.h"

using std::unique_ptr;
using std::vector;
using namespace ue2;

struct hlmMatchEntry {
    size_t to;
    u32 id;
    hlmMatchEntry(size_t end, u32 identifier) :
            to(end), id(identifier) {}
};

vector<hlmMatchEntry> ctxt;

static
hwlmcb_rv_t hlmSimpleCallback(size_t to, u32 id,
                              UNUSED struct hs_scratch *scratch) {
    DEBUG_PRINTF("match @%zu = %u\n", to, id);

    ctxt.push_back(hlmMatchEntry(to, id));

    return HWLM_CONTINUE_MATCHING;
}

static
void noodleMatch(const u8 *data, size_t data_len, const char *lit_str,
                 size_t lit_len, char nocase, HWLMCallback cb) {
    u32 id = 1000;
    hwlmLiteral lit(std::string(lit_str, lit_len), nocase, id);
    auto n = noodBuildTable(lit);
    ASSERT_TRUE(static_cast<bool>(n));

    hwlm_error_t rv;
    struct hs_scratch scratch;
    rv = noodExec(n.get(), data, data_len, 0, cb, &scratch);
    ASSERT_EQ(HWLM_SUCCESS, rv);
}

TEST(Noodle, nood1) {
    const size_t data_len = 1024;
    unsigned int i, j;
    u8 data[data_len];

    memset(data, 'a', data_len);

    noodleMatch(data, data_len, "a", 1, 0, hlmSimpleCallback);
    ASSERT_EQ(1024U, ctxt.size());
    for (i = 0; i < 1024; i++) {
        ASSERT_EQ(i, ctxt[i].to);
    }

    ctxt.clear();
    noodleMatch(data, data_len, "A", 1, 0, hlmSimpleCallback);
    ASSERT_EQ(0U, ctxt.size());

    ctxt.clear();
    noodleMatch(data, data_len, "A", 1, 1, hlmSimpleCallback);
    ASSERT_EQ(1024U, ctxt.size());
    for (i = 0; i < 1024; i++) {
        ASSERT_EQ(i, ctxt[i].to);
    }

    for (j = 0; j < 16; j++) {
        ctxt.clear();
        noodleMatch(data + j, data_len - j, "A", 1, 1, hlmSimpleCallback);
        ASSERT_EQ(1024 - j, ctxt.size());
        for (i = 0; i < 1024 - j; i++) {
            ASSERT_EQ(i, ctxt[i].to);
        }

        ctxt.clear();
        noodleMatch(data, data_len - j, "A", 1, 1, hlmSimpleCallback);
        ASSERT_EQ(1024 - j, ctxt.size());
        for (i = 0; i < 1024 - j; i++) {
            ASSERT_EQ(i, ctxt[i].to);
        }
    }
    ctxt.clear();
}

TEST(Noodle, nood2) {
    const size_t data_len = 1024;
    unsigned int i, j;
    u8 ALIGN_ATTR(32) data[data_len];

    memset(data, 'a', data_len);

    noodleMatch(data, data_len, "aa", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(1023U, ctxt.size());
    for (i = 0; i < 1023; i++) {
        ASSERT_EQ(i + 1, ctxt[i].to);
    }

    ctxt.clear();
    noodleMatch(data, data_len, "aA", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(0U, ctxt.size());

    ctxt.clear();
    noodleMatch(data, data_len, "AA", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(0U, ctxt.size());

    ctxt.clear();
    noodleMatch(data, data_len, "aa", 2, 1, hlmSimpleCallback);
    ASSERT_EQ(1023U, ctxt.size());
    for (i = 0; i < 1023; i++) {
        ASSERT_EQ(i + 1, ctxt[i].to);
    }

    ctxt.clear();
    noodleMatch(data, data_len, "Aa", 2, 1, hlmSimpleCallback);
    ASSERT_EQ(1023U, ctxt.size());
    for (i = 0; i < 1023; i++) {
        ASSERT_EQ(i + 1, ctxt[i].to);
    }

    ctxt.clear();
    noodleMatch(data, data_len, "AA", 2, 1, hlmSimpleCallback);
    ASSERT_EQ(1023U, ctxt.size());
    for (i = 0; i < 1023; i++) {
        ASSERT_EQ(i + 1, ctxt[i].to);
    }

    for (j = 0; j < 16; j++) {
        ctxt.clear();
        noodleMatch(data + j, data_len - j, "Aa", 2, 1, hlmSimpleCallback);
        ASSERT_EQ(1023 - j, ctxt.size());
        for (i = 0; i < 1023 - j; i++) {
            ASSERT_EQ(i + 1, ctxt[i].to);
        }

        ctxt.clear();
        noodleMatch(data, data_len - j, "aA", 2, 1, hlmSimpleCallback);
        ASSERT_EQ(1023 - j, ctxt.size());
        for (i = 0; i < 1023 - j; i++) {
            ASSERT_EQ(i + 1, ctxt[i].to);
        }
    }
    ctxt.clear();
}

TEST(Noodle, noodLong) {
    const size_t data_len = 1024;
    unsigned int i, j;
    u8 data[data_len];

    memset(data, 'a', data_len);

    noodleMatch(data, data_len, "aaaa", 4, 0, hlmSimpleCallback);
    ASSERT_EQ(1021U, ctxt.size());
    for (i = 0; i < 1021; i++) {
        ASSERT_EQ(i + 3, ctxt[i].to);
    }

    ctxt.clear();
    noodleMatch(data, data_len, "aaAA", 4, 0, hlmSimpleCallback);
    ASSERT_EQ(0U, ctxt.size());

    ctxt.clear();
    noodleMatch(data, data_len, "aaAA", 4, 1, hlmSimpleCallback);
    ASSERT_EQ(1021U, ctxt.size());
    for (i = 0; i < 1021; i++) {
        ASSERT_EQ(i + 3, ctxt[i].to);
    }

    for (j = 0; j < 16; j++) {
        ctxt.clear();
        noodleMatch(data + j, data_len - j, "AAaa", 4, 1, hlmSimpleCallback);
        ASSERT_EQ(1021 - j, ctxt.size());
        for (i = 0; i < 1021 - j; i++) {
            ASSERT_EQ(i + 3, ctxt[i].to);
        }

        ctxt.clear();
        noodleMatch(data + j, data_len - j, "aaaA", 4, 1, hlmSimpleCallback);
        ASSERT_EQ(1021 - j, ctxt.size());
        for (i = 0; i < 1021 - j; i++) {
            ASSERT_EQ(i + 3, ctxt[i].to);
        }
    }
    ctxt.clear();
}

TEST(Noodle, noodCutoverSingle) {
    const size_t max_data_len = 128;
    u8 ALIGN_ATTR(32) data[max_data_len + 15];

    memset(data, 'a', max_data_len + 15);

    for (u32 align = 0; align < 16; align++) {
        for (u32 len = 0; len < max_data_len; len++) {
            ctxt.clear();
            noodleMatch(data + align, len, "a", 1, 0, hlmSimpleCallback);
            EXPECT_EQ(len, ctxt.size());
            for (u32 i = 0; i < ctxt.size(); i++) {
                ASSERT_EQ(i, ctxt[i].to);
            }
        }
    }
    ctxt.clear();
}

TEST(Noodle, noodCutoverDouble) {
    const size_t max_data_len = 128;
    u8 data[max_data_len + 15];

    memset(data, 'a', max_data_len + 15);

    for (u32 align = 0; align < 16; align++) {
        for (u32 len = 0; len < max_data_len; len++) {
            ctxt.clear();
            noodleMatch(data + align, len, "aa", 2, 0, hlmSimpleCallback);
            EXPECT_EQ(len ? len - 1 : 0U, ctxt.size());
            for (u32 i = 0; i < ctxt.size(); i++) {
                ASSERT_EQ(i + 1, ctxt[i].to);
            }
        }
    }
    ctxt.clear();
}

// --- Additional tests for SVE and general edge case coverage ---

// Test: callback that terminates matching early
static
hwlmcb_rv_t hlmTerminateAfterN(size_t to, u32 id,
                               UNUSED struct hs_scratch *scratch) {
    ctxt.push_back(hlmMatchEntry(to, id));
    if (ctxt.size() >= 3) {
        return HWLM_TERMINATE_MATCHING;
    }
    return HWLM_CONTINUE_MATCHING;
}

TEST(Noodle, noodTerminateSingle) {
    // Fill buffer with 'a' and terminate after 3 matches
    const size_t data_len = 256;
    u8 data[data_len];
    memset(data, 'a', data_len);

    u32 id = 1000;
    hwlmLiteral lit(std::string("a", 1), false, id);
    auto n = noodBuildTable(lit);
    ASSERT_TRUE(static_cast<bool>(n));

    struct hs_scratch scratch;
    hwlm_error_t rv = noodExec(n.get(), data, data_len, 0,
                               hlmTerminateAfterN, &scratch);
    ASSERT_EQ(HWLM_TERMINATED, rv);
    ASSERT_EQ(3U, ctxt.size());
    ctxt.clear();
}

TEST(Noodle, noodTerminateDouble) {
    // Fill buffer with 'a' and terminate after 3 double matches
    const size_t data_len = 256;
    u8 data[data_len];
    memset(data, 'a', data_len);

    u32 id = 1000;
    hwlmLiteral lit(std::string("aa", 2), false, id);
    auto n = noodBuildTable(lit);
    ASSERT_TRUE(static_cast<bool>(n));

    struct hs_scratch scratch;
    hwlm_error_t rv = noodExec(n.get(), data, data_len, 0,
                               hlmTerminateAfterN, &scratch);
    ASSERT_EQ(HWLM_TERMINATED, rv);
    ASSERT_EQ(3U, ctxt.size());
    ctxt.clear();
}

// Test: no match at all
TEST(Noodle, noodNoMatchSingle) {
    const size_t data_len = 512;
    u8 data[data_len];
    memset(data, 'b', data_len);

    noodleMatch(data, data_len, "a", 1, 0, hlmSimpleCallback);
    ASSERT_EQ(0U, ctxt.size());
    ctxt.clear();
}

TEST(Noodle, noodNoMatchDouble) {
    const size_t data_len = 512;
    u8 data[data_len];
    memset(data, 'b', data_len);

    noodleMatch(data, data_len, "ac", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(0U, ctxt.size());
    ctxt.clear();
}

// Test: very short buffers (edge cases for scan_len checks)
TEST(Noodle, noodShortBufferSingle) {
    u8 data[1] = {'x'};

    noodleMatch(data, 1, "x", 1, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(0U, ctxt[0].to);
    ctxt.clear();

    noodleMatch(data, 1, "y", 1, 0, hlmSimpleCallback);
    ASSERT_EQ(0U, ctxt.size());
    ctxt.clear();
}

TEST(Noodle, noodShortBufferDouble) {
    // 2 bytes: minimum for a double scan
    u8 data2[2] = {'a', 'b'};
    noodleMatch(data2, 2, "ab", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(1U, ctxt[0].to);
    ctxt.clear();

    // 3 bytes
    u8 data3[3] = {'a', 'b', 'c'};
    noodleMatch(data3, 3, "bc", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(2U, ctxt[0].to);
    ctxt.clear();

    // 2 bytes, no match
    noodleMatch(data2, 2, "ba", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(0U, ctxt.size());
    ctxt.clear();
}

// Test: NUL byte in patterns (exercises key1 == '\0' path in scanDoubleOnce)
TEST(Noodle, noodNulByteSingle) {
    const size_t data_len = 64;
    u8 data[data_len];
    memset(data, 'a', data_len);
    data[10] = '\0';
    data[30] = '\0';

    noodleMatch(data, data_len, "\0", 1, 0, hlmSimpleCallback);
    ASSERT_EQ(2U, ctxt.size());
    ASSERT_EQ(10U, ctxt[0].to);
    ASSERT_EQ(30U, ctxt[1].to);
    ctxt.clear();
}

TEST(Noodle, noodNulByteDouble) {
    const size_t data_len = 64;
    u8 data[data_len];
    memset(data, 'a', data_len);
    // Create "a\0" pattern at position 10 and 30
    data[11] = '\0';
    data[31] = '\0';

    noodleMatch(data, data_len, "a\0", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(2U, ctxt.size());
    ASSERT_EQ(11U, ctxt[0].to);
    ASSERT_EQ(31U, ctxt[1].to);
    ctxt.clear();
}

TEST(Noodle, noodNulByteDoubleReverse) {
    // Pattern: "\0a"
    const size_t data_len = 64;
    u8 data[data_len];
    memset(data, 'a', data_len);
    data[10] = '\0';
    data[30] = '\0';

    noodleMatch(data, data_len, "\0a", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(2U, ctxt.size());
    ASSERT_EQ(11U, ctxt[0].to);
    ASSERT_EQ(31U, ctxt[1].to);
    ctxt.clear();
}

// Test: non-alphabetic characters with noCase flag
TEST(Noodle, noodNonAlphaNoCase) {
    const size_t data_len = 128;
    u8 data[data_len];
    memset(data, '1', data_len);

    // noCase should have no effect on digits
    noodleMatch(data, data_len, "1", 1, 1, hlmSimpleCallback);
    ASSERT_EQ(128U, ctxt.size());
    ctxt.clear();

    // Non-alpha double
    memset(data, '!', data_len);
    data[0] = '#';
    noodleMatch(data, data_len, "#!", 2, 1, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(1U, ctxt[0].to);
    ctxt.clear();
}

// Test: case-insensitive double matching
TEST(Noodle, noodDoubleCaseInsensitive) {
    const size_t data_len = 128;
    u8 data[data_len];

    // "aB" pattern should match "ab", "aB", "Ab", "AB" with noCase
    memset(data, 'x', data_len);
    data[10] = 'a'; data[11] = 'b';
    data[20] = 'A'; data[21] = 'B';
    data[30] = 'a'; data[31] = 'B';
    data[40] = 'A'; data[41] = 'b';

    noodleMatch(data, data_len, "aB", 2, 1, hlmSimpleCallback);
    ASSERT_EQ(4U, ctxt.size());
    ASSERT_EQ(11U, ctxt[0].to);
    ASSERT_EQ(21U, ctxt[1].to);
    ASSERT_EQ(31U, ctxt[2].to);
    ASSERT_EQ(41U, ctxt[3].to);
    ctxt.clear();

    // Without noCase, only exact match
    noodleMatch(data, data_len, "aB", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(31U, ctxt[0].to);
    ctxt.clear();
}

// Test: various buffer sizes to exercise scanLoop vs scanOnce paths
// This is especially important for SVE where vector length varies
TEST(Noodle, noodVariousSizesSingle) {
    for (u32 len = 1; len <= 512; len++) {
        std::vector<u8> data(len, 'z');

        ctxt.clear();
        noodleMatch(data.data(), len, "z", 1, 0, hlmSimpleCallback);
        EXPECT_EQ(len, ctxt.size()) << "Failed at len=" << len;
    }
    ctxt.clear();
}

TEST(Noodle, noodVariousSizesDouble) {
    for (u32 len = 2; len <= 512; len++) {
        std::vector<u8> data(len, 'z');

        ctxt.clear();
        noodleMatch(data.data(), len, "zz", 2, 0, hlmSimpleCallback);
        EXPECT_EQ(len - 1, ctxt.size()) << "Failed at len=" << len;
    }
    ctxt.clear();
}

// Test: match at the very end of buffer
TEST(Noodle, noodMatchAtEnd) {
    const size_t data_len = 128;
    u8 data[data_len];
    memset(data, 'x', data_len);
    data[data_len - 1] = 'y';

    noodleMatch(data, data_len, "y", 1, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(data_len - 1, ctxt[0].to);
    ctxt.clear();

    // Double at end
    data[data_len - 2] = 'a';
    data[data_len - 1] = 'b';
    noodleMatch(data, data_len, "ab", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(data_len - 1, ctxt[0].to);
    ctxt.clear();
}

// Test: match at the very beginning of buffer
TEST(Noodle, noodMatchAtStart) {
    const size_t data_len = 128;
    u8 data[data_len];
    memset(data, 'x', data_len);
    data[0] = 'y';

    noodleMatch(data, data_len, "y", 1, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(0U, ctxt[0].to);
    ctxt.clear();

    // Double at start
    data[0] = 'a';
    data[1] = 'b';
    noodleMatch(data, data_len, "ab", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(1U, ctxt[0].to);
    ctxt.clear();
}

// Test: single match in the middle of a large buffer (exercises loop path)
TEST(Noodle, noodSingleMatchLargeBuffer) {
    const size_t data_len = 4096;
    std::vector<u8> data(data_len, 'x');
    data[2048] = 'y';

    noodleMatch(data.data(), data_len, "y", 1, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(2048U, ctxt[0].to);
    ctxt.clear();
}

TEST(Noodle, noodDoubleMatchLargeBuffer) {
    const size_t data_len = 4096;
    std::vector<u8> data(data_len, 'x');
    data[2048] = 'a';
    data[2049] = 'b';

    noodleMatch(data.data(), data_len, "ab", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(1U, ctxt.size());
    ASSERT_EQ(2049U, ctxt[0].to);
    ctxt.clear();
}

// Test: alignment sweep for double scan with NUL byte
// This is critical for the SVE scanDoubleOnce key1=='\0' logic
TEST(Noodle, noodNulByteCutoverDouble) {
    const size_t max_data_len = 256;
    u8 data[max_data_len + 16];
    memset(data, 'z', max_data_len + 16);

    for (u32 align = 0; align < 16; align++) {
        for (u32 len = 3; len < max_data_len; len++) {
            // Place a "z\0" at position len-2
            u8 *base = data + align;
            memset(base, 'z', len);
            base[len - 1] = '\0';

            ctxt.clear();
            noodleMatch(base, len, "z\0", 2, 0, hlmSimpleCallback);
            EXPECT_EQ(1U, ctxt.size())
                << "align=" << align << " len=" << len;
            if (ctxt.size() == 1) {
                EXPECT_EQ(len - 1, ctxt[0].to)
                    << "align=" << align << " len=" << len;
            }

            // Restore
            base[len - 1] = 'z';
        }
    }
    ctxt.clear();
}

// Test: streaming mode with double match spanning history and current buffer
TEST(Noodle, noodStreamingDouble) {
    u32 id = 1000;
    hwlmLiteral lit(std::string("ab", 2), false, id);
    auto n = noodBuildTable(lit);
    ASSERT_TRUE(static_cast<bool>(n));

    // 'a' at end of history, 'b' at start of current
    u8 hbuf[4] = {'x', 'x', 'x', 'a'};
    u8 buf[4] = {'b', 'x', 'x', 'x'};

    struct hs_scratch scratch;
    hwlm_error_t rv = noodExecStreaming(n.get(), hbuf, 4, buf, 4,
                                        hlmSimpleCallback, &scratch);
    ASSERT_EQ(HWLM_SUCCESS, rv);
    ASSERT_EQ(1U, ctxt.size());
    // The match end position should be 0 (first byte of buf)
    ASSERT_EQ(0U, ctxt[0].to);
    ctxt.clear();
}

// Test: streaming mode - no match across boundary
TEST(Noodle, noodStreamingNoMatch) {
    u32 id = 1000;
    hwlmLiteral lit(std::string("ab", 2), false, id);
    auto n = noodBuildTable(lit);
    ASSERT_TRUE(static_cast<bool>(n));

    u8 hbuf[4] = {'x', 'x', 'x', 'x'};
    u8 buf[4] = {'x', 'x', 'x', 'x'};

    struct hs_scratch scratch;
    hwlm_error_t rv = noodExecStreaming(n.get(), hbuf, 4, buf, 4,
                                        hlmSimpleCallback, &scratch);
    ASSERT_EQ(HWLM_SUCCESS, rv);
    ASSERT_EQ(0U, ctxt.size());
    ctxt.clear();
}

// Test: offset parameter to start scanning from a later position
TEST(Noodle, noodWithOffset) {
    const size_t data_len = 128;
    u8 data[data_len];
    memset(data, 'a', data_len);

    // Start scanning from offset 64
    u32 id = 1000;
    hwlmLiteral lit(std::string("a", 1), false, id);
    auto n = noodBuildTable(lit);
    ASSERT_TRUE(static_cast<bool>(n));

    struct hs_scratch scratch;
    hwlm_error_t rv = noodExec(n.get(), data, data_len, 64,
                               hlmSimpleCallback, &scratch);
    ASSERT_EQ(HWLM_SUCCESS, rv);
    ASSERT_EQ(64U, ctxt.size());
    ASSERT_EQ(64U, ctxt[0].to);
    ctxt.clear();
}

// Test: long pattern (4+ chars) with double scan path
TEST(Noodle, noodLongPatternCutover) {
    const size_t max_data_len = 256;
    u8 data[max_data_len + 16];
    memset(data, 'a', max_data_len + 16);

    for (u32 align = 0; align < 16; align++) {
        for (u32 len = 4; len < max_data_len; len++) {
            ctxt.clear();
            noodleMatch(data + align, len, "aaaa", 4, 0, hlmSimpleCallback);
            EXPECT_EQ(len - 3, ctxt.size())
                << "align=" << align << " len=" << len;
        }
    }
    ctxt.clear();
}

// Test: repeated pattern with multiple matches in single vector
TEST(Noodle, noodDenseMatches) {
    // Alternating 'ab' pattern creates dense double matches
    const size_t data_len = 256;
    u8 data[data_len];
    for (size_t i = 0; i < data_len; i++) {
        data[i] = (i % 2 == 0) ? 'a' : 'b';
    }

    noodleMatch(data, data_len, "ab", 2, 0, hlmSimpleCallback);
    ASSERT_EQ(128U, ctxt.size());
    for (u32 i = 0; i < 128; i++) {
        ASSERT_EQ(i * 2 + 1, ctxt[i].to);
    }
    ctxt.clear();
}

