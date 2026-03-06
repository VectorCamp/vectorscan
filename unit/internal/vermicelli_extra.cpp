/*
 * Copyright (c) 2026, AhnLab, Inc.
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

/**
 * Additional unit tests for vermicelli accelerator functions.
 *
 * Focuses on:
 * - Small buffer edge cases (below VECTORSIZE)
 * - Exact VECTORSIZE boundary
 * - Single-byte buffers
 * - Per-position match verification
 * - Partial match semantics for double vermicelli
 * - Missing reverse double NoMatch tests
 * - Non-alphabetic characters
 */

#include "config.h"

#include "gtest/gtest.h"
#include "nfa/vermicelli.hpp"
#include "util/bitutils.h"

#include <cstring>
#include <string>
#include <vector>
#include <algorithm>

// ============================================================================
// Forward single-byte vermicelli: small buffer tests
// ============================================================================

TEST(VermicelliExtra, SmallMatchSingleByte) {
    // Buffer of size 1
    const u8 buf[1] = {'x'};
    const u8 *rv = vermicelliExec('x', 0, buf, buf + 1);
    EXPECT_EQ(buf, rv);
}

TEST(VermicelliExtra, SmallNoMatchSingleByte) {
    const u8 buf[1] = {'y'};
    const u8 *rv = vermicelliExec('x', 0, buf, buf + 1);
    EXPECT_EQ(buf + 1, rv);
}

TEST(VermicelliExtra, SmallMatchTwoBytes) {
    // Match at position 0
    const u8 buf1[2] = {'x', 'y'};
    EXPECT_EQ(buf1, vermicelliExec('x', 0, buf1, buf1 + 2));
    // Match at position 1
    const u8 buf2[2] = {'y', 'x'};
    EXPECT_EQ(buf2 + 1, vermicelliExec('x', 0, buf2, buf2 + 2));
}

TEST(VermicelliExtra, SmallNoCaseMatch) {
    // Searching uppercase with nocase in a 3-byte buffer containing lowercase
    const u8 buf[3] = {'z', 'a', 'z'};
    const u8 *rv = vermicelliExec('A', 1, buf, buf + 3);
    EXPECT_EQ(buf + 1, rv);
}

TEST(VermicelliExtra, SmallRangeSweep) {
    // For each buffer size from 1 to VECTORSIZE-1, place the target at
    // every position and verify it is found correctly.
    for (int len = 1; len < 32; len++) {
        SCOPED_TRACE(len);
        std::vector<u8> buf(len, 'b');
        for (int pos = 0; pos < len; pos++) {
            SCOPED_TRACE(pos);
            buf[pos] = 'a';
            const u8 *rv = vermicelliExec('a', 0, buf.data(),
                                          buf.data() + len);
            EXPECT_EQ(buf.data() + pos, rv);
            buf[pos] = 'b';  // cppcheck-suppress unreadVariable // restore
        }
    }
}

// ============================================================================
// Exact VECTORSIZE boundary tests
// ============================================================================

TEST(VermicelliExtra, ExactVecSizeNoMatch) {
    // Buffer of exactly 32 bytes with no match
    std::vector<u8> buf(32, 'b');
    const u8 *rv = vermicelliExec('a', 0, buf.data(), buf.data() + 32);
    EXPECT_EQ(buf.data() + 32, rv);
}

TEST(VermicelliExtra, ExactVecSizeMatchFirst) {
    std::vector<u8> buf(32, 'b');
    buf[0] = 'a';
    const u8 *rv = vermicelliExec('a', 0, buf.data(), buf.data() + 32);
    EXPECT_EQ(buf.data(), rv);
}

TEST(VermicelliExtra, ExactVecSizeMatchLast) {
    std::vector<u8> buf(32, 'b');
    buf[31] = 'a';
    const u8 *rv = vermicelliExec('a', 0, buf.data(), buf.data() + 32);
    EXPECT_EQ(buf.data() + 31, rv);
}

TEST(VermicelliExtra, ExactVecSizePlusOne) {
    // VECTORSIZE + 1 bytes, match at last position (exercises tail path)
    std::vector<u8> buf(33, 'b');
    buf[32] = 'a';
    const u8 *rv = vermicelliExec('a', 0, buf.data(), buf.data() + 33);
    EXPECT_EQ(buf.data() + 32, rv);
}

// ============================================================================
// Per-position match within SIMD register
// ============================================================================

TEST(VermicelliExtra, PerPositionForward) {
    // Large buffer, sweep match through every position
    const size_t LEN = 128;
    for (size_t pos = 0; pos < LEN; pos++) {
        SCOPED_TRACE(pos);
        std::vector<u8> buf(LEN, 'b');
        buf[pos] = 'a';
        const u8 *rv = vermicelliExec('a', 0, buf.data(),
                                      buf.data() + LEN);
        EXPECT_EQ(buf.data() + pos, rv);
    }
}

TEST(VermicelliExtra, PerPositionReverse) {
    // Large buffer, sweep match through every position for reverse search
    const size_t LEN = 128;
    for (size_t pos = 0; pos < LEN; pos++) {
        SCOPED_TRACE(pos);
        std::vector<u8> buf(LEN, 'b');
        buf[pos] = 'a';
        const u8 *rv = rvermicelliExec('a', 0, buf.data(),
                                       buf.data() + LEN);
        EXPECT_EQ(buf.data() + pos, rv);
    }
}

// ============================================================================
// Non-alphabetic character tests
// ============================================================================

TEST(VermicelliExtra, NonAlphaChars) {
    //                01234567890123456
    const u8 buf[] = "hello, world! 123";
    size_t len = strlen(reinterpret_cast<const char *>(buf));

    // Space character at position 6
    const u8 *rv = vermicelliExec(' ', 0, buf, buf + len);
    EXPECT_EQ(buf + 6, rv);

    // Digit '1' at position 14
    rv = vermicelliExec('1', 0, buf, buf + len);
    EXPECT_EQ(buf + 14, rv);

    // Exclamation mark at position 12
    rv = vermicelliExec('!', 0, buf, buf + len);
    EXPECT_EQ(buf + 12, rv);

    // Comma at position 5
    rv = vermicelliExec(',', 0, buf, buf + len);
    EXPECT_EQ(buf + 5, rv);
}

// ============================================================================
// NVermicelli (negated) small buffer tests
// ============================================================================

TEST(NVermicelliExtra, SmallSingleByte) {
    // 1-byte buffer, byte matches → buf_end
    const u8 buf1[1] = {'a'};
    EXPECT_EQ(buf1 + 1, nvermicelliExec('a', 0, buf1, buf1 + 1));

    // 1-byte buffer, byte doesn't match → position 0
    const u8 buf2[1] = {'b'};
    EXPECT_EQ(buf2, nvermicelliExec('a', 0, buf2, buf2 + 1));
}

TEST(NVermicelliExtra, SmallRangeSweep) {
    // Buffer of matching chars with one different char at each position
    for (int len = 1; len < 32; len++) {
        SCOPED_TRACE(len);
        std::vector<u8> buf(len, 'a');
        for (int pos = 0; pos < len; pos++) {
            SCOPED_TRACE(pos);
            buf[pos] = 'z';
            const u8 *rv = nvermicelliExec('a', 0, buf.data(),
                                           buf.data() + len);
            EXPECT_EQ(buf.data() + pos, rv);
            buf[pos] = 'a';  // cppcheck-suppress unreadVariable
        }
    }
}

TEST(NVermicelliExtra, NoCaseSmall) {
    // nocase: both 'a' and 'A' should be considered 'the char'
    const u8 buf[4] = {'a', 'A', 'a', 'b'};
    const u8 *rv = nvermicelliExec('A', 1, buf, buf + 4);
    EXPECT_EQ(buf + 3, rv);  // 'b' is the first non-matching
}

// ============================================================================
// Reverse vermicelli small buffer tests
// ============================================================================

TEST(RVermicelliExtra, SmallSingleByte) {
    const u8 buf[1] = {'x'};
    EXPECT_EQ(buf, rvermicelliExec('x', 0, buf, buf + 1));

    const u8 raw2[2] = {0, 'y'};
    const u8 *buf2 = raw2 + 1;
    EXPECT_EQ(raw2, rvermicelliExec('x', 0, buf2, buf2 + 1));
}

TEST(RVermicelliExtra, SmallThreeBytes) {
    const u8 buf[3] = {'a', 'b', 'a'};
    // Reverse should find the LAST 'a' at position 2
    const u8 *rv = rvermicelliExec('a', 0, buf, buf + 3);
    EXPECT_EQ(buf + 2, rv);

    // Reverse find last 'b' at position 1
    rv = rvermicelliExec('b', 0, buf, buf + 3);
    EXPECT_EQ(buf + 1, rv);
}

TEST(RVermicelliExtra, SmallRangeSweep) {
    for (int len = 1; len < 32; len++) {
        SCOPED_TRACE(len);
        std::vector<u8> buf(len, 'b');
        for (int pos = 0; pos < len; pos++) {
            SCOPED_TRACE(pos);
            buf[pos] = 'a';
            const u8 *rv = rvermicelliExec('a', 0, buf.data(),
                                           buf.data() + len);
            // Reverse returns the LAST match; 'a' is the only one at 'pos'
            EXPECT_EQ(buf.data() + pos, rv);
            buf[pos] = 'b';  // cppcheck-suppress unreadVariable
        }
    }
}

// ============================================================================
// Reverse negated vermicelli small buffer tests
// ============================================================================

TEST(RNVermicelliExtra, SmallSingleByte) {
    const u8 raw[2] = {0, 'a'};
    const u8 *buf = raw + 1;
    // All match → not found → raw (i.e. buf - 1)
    EXPECT_EQ(raw, rnvermicelliExec('a', 0, buf, buf + 1));

    const u8 buf2[1] = {'b'};
    // Doesn't match → found at 0
    EXPECT_EQ(buf2, rnvermicelliExec('a', 0, buf2, buf2 + 1));
}

TEST(RNVermicelliExtra, SmallRangeSweep) {
    for (int len = 1; len < 32; len++) {
        SCOPED_TRACE(len);
        std::vector<u8> buf(len, 'a');
        for (int pos = 0; pos < len; pos++) {
            SCOPED_TRACE(pos);
            buf[pos] = 'z';
            const u8 *rv = rnvermicelliExec('a', 0, buf.data(),
                                            buf.data() + len);
            // Reverse negated finds the LAST non-matching byte
            EXPECT_EQ(buf.data() + pos, rv);
            buf[pos] = 'a';  // cppcheck-suppress unreadVariable
        }
    }
}

// ============================================================================
// Double vermicelli: small buffer / edge case tests
// ============================================================================

TEST(DoubleVermicelliExtra, TwoByteBuffer) {
    const u8 buf[2] = {'a', 'b'};
    const u8 *rv = vermicelliDoubleExec('a', 'b', 0, buf, buf + 2);
    EXPECT_EQ(buf, rv);

    // No match
    rv = vermicelliDoubleExec('b', 'a', 0, buf, buf + 2);
    // 'b' at end → partial match
    EXPECT_EQ(buf + 1, rv);  // last byte matches c1
}

TEST(DoubleVermicelliExtra, TwoByteNoFullMatch) {
    const u8 buf[2] = {'x', 'y'};
    const u8 *rv = vermicelliDoubleExec('a', 'b', 0, buf, buf + 2);
    EXPECT_EQ(buf + 2, rv);
}

TEST(DoubleVermicelliExtra, ThreeByteMatchMiddle) {
    const u8 buf[3] = {'x', 'a', 'b'};
    const u8 *rv = vermicelliDoubleExec('a', 'b', 0, buf, buf + 3);
    EXPECT_EQ(buf + 1, rv);
}

TEST(DoubleVermicelliExtra, SmallRangeSweep) {
    // Sweep a double match through small buffers
    for (int len = 2; len < 32; len++) {
        SCOPED_TRACE(len);
        for (int pos = 0; pos < len - 1; pos++) {
            SCOPED_TRACE(pos);
            std::vector<u8> buf(len, 'z');
            buf[pos] = 'a';
            buf[pos + 1] = 'b';
            const u8 *rv = vermicelliDoubleExec('a', 'b', 0, buf.data(),
                                                buf.data() + len);
            EXPECT_EQ(buf.data() + pos, rv);
        }
    }
}

TEST(DoubleVermicelliExtra, PartialMatchAtEnd) {
    // The last byte matches c1 but there is no c2 after it
    const u8 buf[] = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxa";
    size_t len = strlen(reinterpret_cast<const char *>(buf));
    const u8 *rv = vermicelliDoubleExec('a', 'b', 0, buf, buf + len);
    EXPECT_EQ(buf + len - 1, rv);  // Partial match at the very end
}

TEST(DoubleVermicelliExtra, NoCaseSmall) {
    // nocase with small buffer
    const u8 buf[3] = {'x', 'A', 'B'};
    const u8 *rv = vermicelliDoubleExec('A', 'B', 1, buf, buf + 3);
    EXPECT_EQ(buf + 1, rv);

    const u8 buf2[3] = {'x', 'a', 'b'};
    rv = vermicelliDoubleExec('A', 'B', 1, buf2, buf2 + 3);
    EXPECT_EQ(buf2 + 1, rv);
}

TEST(DoubleVermicelliExtra, ExactVecSize) {
    std::vector<u8> buf(32, 'z');
    buf[30] = 'a';
    buf[31] = 'b';
    const u8 *rv = vermicelliDoubleExec('a', 'b', 0, buf.data(),
                                        buf.data() + 32);
    EXPECT_EQ(buf.data() + 30, rv);
}

TEST(DoubleVermicelliExtra, ExactVecSizePlusOne) {
    // Match at the very end, in the tail path
    std::vector<u8> buf(33, 'z');
    buf[31] = 'a';
    buf[32] = 'b';
    const u8 *rv = vermicelliDoubleExec('a', 'b', 0, buf.data(),
                                        buf.data() + 33);
    EXPECT_EQ(buf.data() + 31, rv);
}

// ============================================================================
// Reverse double vermicelli: NoMatch tests (were missing)
// ============================================================================

TEST(RDoubleVermicelliExtra, ExecNoMatch1) {
    char t0[] = " bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char *t1 = t0 + 1;

    for (size_t i = 0; i < 16; i++) {
        for (size_t j = 0; j < 16; j++) {
            SCOPED_TRACE(i);
            SCOPED_TRACE(j);
            const u8 *begin = reinterpret_cast<const u8 *>(t1) + i;
            const u8 *end = reinterpret_cast<const u8 *>(t1) + strlen(t1) - j;
            if (begin >= end) continue;

            const u8 *before_begin = begin - 1;
            const u8 *rv = rvermicelliDoubleExec('a', 'b', 0, begin, end);
            ASSERT_EQ(before_begin, rv);

            rv = rvermicelliDoubleExec('B', 'B', 0, begin, end);
            ASSERT_EQ(before_begin, rv);

            rv = rvermicelliDoubleExec('A', 'B', 1, begin, end);
            ASSERT_EQ(before_begin, rv);
        }
    }
}

// ============================================================================
// Reverse double vermicelli: small buffer tests
// ============================================================================

TEST(RDoubleVermicelliExtra, TwoByteBuffer) {
    const u8 buf[2] = {'a', 'b'};
    // rvermicelliDoubleExec returns position of c2
    const u8 *rv = rvermicelliDoubleExec('a', 'b', 0, buf, buf + 2);
    EXPECT_EQ(buf + 1, rv);

    // No match in reverse for 'b','a'
    const u8 raw2[3] = {0, 'a', 'b'};
    const u8 *buf2 = raw2 + 1;
    rv = rvermicelliDoubleExec('b', 'a', 0, buf2, buf2 + 2);
    EXPECT_EQ(raw2, rv);
}

TEST(RDoubleVermicelliExtra, SmallRangeSweep) {
    for (int len = 2; len < 32; len++) {
        SCOPED_TRACE(len);
        for (int pos = 0; pos < len - 1; pos++) {
            SCOPED_TRACE(pos);
            std::vector<u8> buf(len, 'z');
            buf[pos] = 'a';
            buf[pos + 1] = 'b';
            // Only match at (pos, pos+1), reverse finds it
            const u8 *rv = rvermicelliDoubleExec('a', 'b', 0, buf.data(),
                                                 buf.data() + len);
            EXPECT_EQ(buf.data() + pos + 1, rv);  // returns c2 position
        }
    }
}

TEST(RDoubleVermicelliExtra, MultipleMatchesReverse) {
    // Multiple double matches, reverse should find the last one
    //                0123456789
    const u8 buf[] = "xxaBxxaBxx";
    size_t len = 10;
    const u8 *rv = rvermicelliDoubleExec('a', 'B', 0, buf, buf + len);
    EXPECT_EQ(buf + 7, rv);  // Last 'B' (c2) in 'aB' pair at pos 6,7
}

// ============================================================================
// Double vermicelli masked: small buffer tests
// ============================================================================
#ifndef HAVE_SVE2
// vermicelliDoubleMaskedExec() for small buffer is not implemented with SVE
TEST(DoubleVermicelliMaskedExtra, TwoByteBuffer) {
    const u8 buf[2] = {'a', 'b'};
    const u8 *rv = vermicelliDoubleMaskedExec('a', 'b', 0xff, 0xff,
                                              buf, buf + 2);
    EXPECT_EQ(buf, rv);
}

TEST(DoubleVermicelliMaskedExtra, SmallRangeSweep) {
    for (int len = 2; len < 32; len++) {
        SCOPED_TRACE(len);
        for (int pos = 0; pos < len - 1; pos++) {
            SCOPED_TRACE(pos);
            std::vector<u8> buf(len, 'z');
            buf[pos] = 'a';
            buf[pos + 1] = 'b';
            const u8 *rv = vermicelliDoubleMaskedExec('a', 'b', 0xff, 0xff,
                                                      buf.data(),
                                                      buf.data() + len);
            EXPECT_EQ(buf.data() + pos, rv);
        }
    }
}

TEST(DoubleVermicelliMaskedExtra, MaskPartialBits) {
    // Use masks to match only certain bits
    // 'a' = 0x61, 'b' = 0x62, 'c' = 0x63
    // With mask 0xfe, 'b' (0x62) and 'c' (0x63) both become 0x62
    const u8 buf[3] = {'a', 'c', 'z'};
    const u8 *rv = vermicelliDoubleMaskedExec('a', 'b', 0xff, 0xfe,
                                              buf, buf + 3);
    // 'a' & 0xff = 'a' matches c1='a', 'c' & 0xfe = 0x62 matches c2='b'=0x62
    EXPECT_EQ(buf, rv);
}
#endif

// ============================================================================
// Alignment stress test: varying start offset and length
// ============================================================================

TEST(VermicelliExtra, AlignmentStress) {
    // Allocate a large buffer and test with various alignments
    const size_t TOTAL = 256;
    std::vector<u8> alloc(TOTAL + 64, 'b');
    u8 *base = alloc.data();

    // Ensure base is aligned to 64 (safe source)
    u8 *aligned_base = reinterpret_cast<u8 *>(
        (reinterpret_cast<uintptr_t>(base) + 63) & ~uintptr_t(63));

    for (size_t offset = 0; offset < 32; offset++) {
        for (size_t len = 1; len <= 128; len++) {
            SCOPED_TRACE(offset);
            SCOPED_TRACE(len);

            u8 *buf = aligned_base + offset;
            std::fill(buf, buf + len, 'b');

            // No match
            const u8 *rv = vermicelliExec('a', 0, buf, buf + len);
            EXPECT_EQ(buf + len, rv);

            // Place match at last position
            buf[len - 1] = 'a';
            rv = vermicelliExec('a', 0, buf, buf + len);
            EXPECT_EQ(buf + len - 1, rv);
            buf[len - 1] = 'b';

            // Place match at first position
            buf[0] = 'a';
            rv = vermicelliExec('a', 0, buf, buf + len);
            EXPECT_EQ(buf, rv);
            buf[0] = 'b';
        }
    }
}

// ============================================================================
// NVermicelli: exact VECTORSIZE boundary
// ============================================================================

TEST(NVermicelliExtra, ExactVecSizeAllMatch) {
    std::vector<u8> buf(32, 'a');
    const u8 *rv = nvermicelliExec('a', 0, buf.data(), buf.data() + 32);
    EXPECT_EQ(buf.data() + 32, rv);
}

TEST(NVermicelliExtra, ExactVecSizeFirstDiff) {
    std::vector<u8> buf(32, 'a');
    buf[0] = 'x';
    const u8 *rv = nvermicelliExec('a', 0, buf.data(), buf.data() + 32);
    EXPECT_EQ(buf.data(), rv);
}

TEST(NVermicelliExtra, ExactVecSizeLastDiff) {
    std::vector<u8> buf(32, 'a');
    buf[31] = 'x';
    const u8 *rv = nvermicelliExec('a', 0, buf.data(), buf.data() + 32);
    EXPECT_EQ(buf.data() + 31, rv);
}

// ============================================================================
// Reverse vermicelli: exact VECTORSIZE boundary
// ============================================================================

TEST(RVermicelliExtra, ExactVecSizeNoMatch) {
    std::vector<u8> buf(33, 'b');
    const u8 *data = buf.data() + 1;
    const u8 *rv = rvermicelliExec('a', 0, data, data + 32);
    EXPECT_EQ(buf.data(), rv);
}

TEST(RVermicelliExtra, ExactVecSizeMatchFirst) {
    std::vector<u8> buf(32, 'b');
    buf[0] = 'a';
    const u8 *rv = rvermicelliExec('a', 0, buf.data(), buf.data() + 32);
    EXPECT_EQ(buf.data(), rv);
}

TEST(RVermicelliExtra, ExactVecSizeMatchLast) {
    std::vector<u8> buf(32, 'b');
    buf[31] = 'a';
    const u8 *rv = rvermicelliExec('a', 0, buf.data(), buf.data() + 32);
    EXPECT_EQ(buf.data() + 31, rv);
}

// ============================================================================
// Double vermicelli: per-position sweep in large buffer
// ============================================================================

TEST(DoubleVermicelliExtra, PerPositionSweep) {
    const size_t LEN = 128;
    for (size_t pos = 0; pos < LEN - 1; pos++) {
        SCOPED_TRACE(pos);
        std::vector<u8> buf(LEN, 'z');
        buf[pos] = 'a';
        buf[pos + 1] = 'b';
        const u8 *rv = vermicelliDoubleExec('a', 'b', 0, buf.data(),
                                            buf.data() + LEN);
        EXPECT_EQ(buf.data() + pos, rv);
    }
}

TEST(RDoubleVermicelliExtra, PerPositionSweep) {
    const size_t LEN = 128;
    for (size_t pos = 0; pos < LEN - 1; pos++) {
        SCOPED_TRACE(pos);
        std::vector<u8> buf(LEN, 'z');
        buf[pos] = 'a';
        buf[pos + 1] = 'b';
        const u8 *rv = rvermicelliDoubleExec('a', 'b', 0, buf.data(),
                                             buf.data() + LEN);
        EXPECT_EQ(buf.data() + pos + 1, rv);  // returns c2 pos
    }
}

// ============================================================================
// NVermicelli: per-position sweep
// ============================================================================

TEST(NVermicelliExtra, PerPositionSweep) {
    const size_t LEN = 128;
    for (size_t pos = 0; pos < LEN; pos++) {
        SCOPED_TRACE(pos);
        std::vector<u8> buf(LEN, 'a');
        buf[pos] = 'x';
        const u8 *rv = nvermicelliExec('a', 0, buf.data(),
                                       buf.data() + LEN);
        EXPECT_EQ(buf.data() + pos, rv);
    }
}

// ============================================================================
// RNVermicelli: per-position sweep
// ============================================================================

TEST(RNVermicelliExtra, PerPositionSweep) {
    const size_t LEN = 128;
    for (size_t pos = 0; pos < LEN; pos++) {
        SCOPED_TRACE(pos);
        std::vector<u8> buf(LEN, 'a');
        buf[pos] = 'x';
        const u8 *rv = rnvermicelliExec('a', 0, buf.data(),
                                        buf.data() + LEN);
        EXPECT_EQ(buf.data() + pos, rv);  // reverse: finds last non-match
    }
}

// ============================================================================
// Double vermicelli masked: per-position sweep
// ============================================================================

TEST(DoubleVermicelliMaskedExtra, PerPositionSweep) {
    const size_t LEN = 128;
    for (size_t pos = 0; pos < LEN - 1; pos++) {
        SCOPED_TRACE(pos);
        std::vector<u8> buf(LEN, 'z');
        buf[pos] = 'a';
        buf[pos + 1] = 'b';
        const u8 *rv = vermicelliDoubleMaskedExec('a', 'b', 0xff, 0xff,
                                                  buf.data(),
                                                  buf.data() + LEN);
        EXPECT_EQ(buf.data() + pos, rv);
    }
}

// ============================================================================
// Cross-boundary double match stress test
// ============================================================================

TEST(DoubleVermicelliExtra, CrossBoundarySweep) {
    // Place a double match right at every potential SIMD boundary
    // VECTORSIZE = 32 in this build
    const size_t LEN = 256;
    for (size_t start = 0; start < 32; start++) {
        SCOPED_TRACE(start);
        std::vector<u8> alloc(LEN + 64, 'z');
        u8 *buf = alloc.data() + start;
        size_t buflen = LEN;

        for (size_t pos = 0; pos < buflen - 1; pos++) {
            buf[pos] = 'a';
            buf[pos + 1] = 'b';

            const u8 *rv = vermicelliDoubleExec('a', 'b', 0, buf,
                                                buf + buflen);
            EXPECT_EQ(buf + pos, rv) << "start=" << start << " pos=" << pos;

            buf[pos] = 'z';
            buf[pos + 1] = 'z';
        }
    }
}

// ============================================================================
// Consistency test: forward + reverse on same data
// ============================================================================

TEST(VermicelliExtra, ForwardReverseConsistency) {
    const size_t LEN = 200;
    std::vector<u8> buf(LEN, 'b');

    // Place a single 'a' at each position
    for (size_t pos = 0; pos < LEN; pos++) {
        SCOPED_TRACE(pos);
        buf[pos] = 'a';

        const u8 *fwd = vermicelliExec('a', 0, buf.data(), buf.data() + LEN);
        const u8 *rev = rvermicelliExec('a', 0, buf.data(), buf.data() + LEN);

        // With only one match, forward and reverse should find the same
        EXPECT_EQ(fwd, rev);
        EXPECT_EQ(buf.data() + pos, fwd);

        buf[pos] = 'b';
    }

    // Two matches: forward finds first, reverse finds last
    buf[10] = 'a';
    buf[190] = 'a';
    const u8 *fwd = vermicelliExec('a', 0, buf.data(), buf.data() + LEN);
    const u8 *rev = rvermicelliExec('a', 0, buf.data(), buf.data() + LEN);
    EXPECT_EQ(buf.data() + 10, fwd);
    EXPECT_EQ(buf.data() + 190, rev);
}

// ============================================================================
// NVermicelli + RNVermicelli consistency
// ============================================================================

TEST(NVermicelliExtra, ForwardReverseConsistency) {
    const size_t LEN = 200;
    std::vector<u8> buf(LEN, 'a');

    // Place a single different byte at each position
    for (size_t pos = 0; pos < LEN; pos++) {
        SCOPED_TRACE(pos);
        buf[pos] = 'x';

        const u8 *fwd = nvermicelliExec('a', 0, buf.data(), buf.data() + LEN);
        const u8 *rev = rnvermicelliExec('a', 0, buf.data(), buf.data() + LEN);

        EXPECT_EQ(fwd, rev);
        EXPECT_EQ(buf.data() + pos, fwd);

        buf[pos] = 'a';
    }

    // Two different bytes
    buf[10] = 'x';
    buf[190] = 'x';
    const u8 *fwd = nvermicelliExec('a', 0, buf.data(), buf.data() + LEN);
    const u8 *rev = rnvermicelliExec('a', 0, buf.data(), buf.data() + LEN);
    EXPECT_EQ(buf.data() + 10, fwd);
    EXPECT_EQ(buf.data() + 190, rev);
}
