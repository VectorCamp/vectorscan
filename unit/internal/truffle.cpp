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

#include "gtest/gtest.h"
#include "nfa/truffle.h"
#include "nfa/trufflecompile.h"
#include "util/charreach.h"
#include "util/simd_utils.h"

#include <array>
#include <cstring>

using namespace ue2;

TEST(Truffle, CompileDot) {
    m128 mask1, mask2;
    memset(&mask1, 0, sizeof(mask1)); // cppcheck-suppress memsetClassFloat
    memset(&mask2, 0, sizeof(mask2)); // cppcheck-suppress memsetClassFloat

    CharReach chars;

    chars.setall();

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    CharReach out = truffle2cr(reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    ASSERT_EQ(out, chars);

}

TEST(Truffle, CompileChars) {
    m128 mask1, mask2;

    CharReach chars;

    // test one char at a time
    for (u32 c = 0; c < 256; ++c) {
        mask1 = zeroes128();
        mask2 = zeroes128();
        chars.clear();
        chars.set((u8)c);
        truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
        CharReach out = truffle2cr(reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
        ASSERT_EQ(out, chars);
    }

    // set all chars up to dot
    for (u32 c = 0; c < 256; ++c) {
        mask1 = zeroes128();
        mask2 = zeroes128();
        chars.set((u8)c);
        truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
        CharReach out = truffle2cr(reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
        ASSERT_EQ(out, chars);
    }

    // unset all chars from dot
    for (u32 c = 0; c < 256; ++c) {
        mask1 = zeroes128();
        mask2 = zeroes128();
        chars.clear((u8)c);
        truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
        CharReach out = truffle2cr(reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
        ASSERT_EQ(out, chars);
    }

}

TEST(Truffle, ExecNoMatch1) {
    m128 mask1, mask2;
    memset(&mask1, 0, sizeof(mask1)); // cppcheck-suppress memsetClassFloat
    memset(&mask2, 0, sizeof(mask2)); // cppcheck-suppress memsetClassFloat

    CharReach chars;

    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\xff";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + strlen(t1), (size_t)rv);
    }
}

TEST(Truffle, ExecNoMatch2) {
    m128 mask1, mask2;

    CharReach chars;

    chars.set('a');
    chars.set('B');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + strlen(t1), (size_t)rv);
    }
}

TEST(Truffle, ExecNoMatch3) {
    m128 mask1, mask2;

    CharReach chars;

    chars.set('V'); /* V = 0x56, e = 0x65 */

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    char t1[] = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + strlen(t1), (size_t)rv);
    }
}

TEST(Truffle, ExecMiniMatch0) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    char t1[] = "a";

    const u8 *rv = truffleExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + strlen(t1));

    ASSERT_EQ((size_t)t1, (size_t)rv);
}

TEST(Truffle, ExecMiniMatch1) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    char t1[] = "bbbbbbbabbb";

    const u8 *rv = truffleExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + strlen(t1));

    ASSERT_EQ((size_t)t1 + 7, (size_t)rv);
}

TEST(Truffle, ExecMiniMatch2) {
    m128 lo, hi;

    CharReach chars;
    chars.set(0);

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    char t1[] = "bbbbbbb\0bbb";

    const u8 *rv = truffleExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + 11);

    ASSERT_EQ((size_t)t1 + 7, (size_t)rv);
}

TEST(Truffle, ExecMiniMatch3) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    char t1[] = "\0\0\0\0\0\0\0a\0\0\0";

    const u8 *rv = truffleExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + 11);

    ASSERT_EQ((size_t)t1 + 7, (size_t)rv);
}

TEST(Truffle, ExecMatchBig) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    std::array<u8, 400> t1;
    t1.fill('b');
    t1[120] = 'a';

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec(lo, hi, reinterpret_cast<u8 *>(t1.data()) + i, reinterpret_cast<u8 *>(t1.data()) + 399);

        ASSERT_LE(((size_t)t1.data() + 120) & ~0xf, (size_t)rv);
    }
}

TEST(Truffle, ExecMatch1) {
    m128 mask1, mask2;

    CharReach chars;

    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(Truffle, ExecMatch2) {
    m128 mask1, mask2;

    CharReach chars;

    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(Truffle, ExecMatch3) {
    m128 mask1, mask2;

    CharReach chars;

    chars.set('a');
    chars.set('B');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbBaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(Truffle, ExecMatch4) {
    m128 mask1, mask2;

    CharReach chars;

    chars.set('a');
    chars.set('C');
    chars.set('A');
    chars.set('c');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbAaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t2[] = "bbbbbbbbbbbbbbbbbCaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t3[] = "bbbbbbbbbbbbbbbbbcaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t4[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);

        rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t2) + i, reinterpret_cast<u8 *>(t2) + strlen(t1));

        ASSERT_EQ((size_t)t2 + 17, (size_t)rv);

        rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t3) + i, reinterpret_cast<u8 *>(t3) + strlen(t3));

        ASSERT_EQ((size_t)t3 + 17, (size_t)rv);

        rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t4) + i, reinterpret_cast<u8 *>(t4) + strlen(t4));

        ASSERT_EQ((size_t)t4 + 17, (size_t)rv);
    }
}

TEST(Truffle, ExecMatch5) {
    m128 mask1, mask2;

    CharReach chars;

    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 31; i++) {
        t1[48 - i] = 'a';
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)&t1[48 - i], (size_t)rv);
    }
}

TEST(Truffle, ExecMatch6) {
    m128 mask1, mask2;

    CharReach chars;

    // [0-Z] - includes some graph chars
    chars.setRange('0', 'Z');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    std::array<u8, 128> t1;
    t1.fill('*'); // it's full of stars!

    for (u8 c = '0'; c <= 'Z'; c++) {
        t1[17] = c;
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1.data()), reinterpret_cast<u8 *>(t1.data()) + 128);

        ASSERT_EQ((size_t)t1.data() + 17, (size_t)rv);
    }
}

TEST(Truffle, ExecMatch7) {
    m128 mask1, mask2;

    CharReach chars;

    // hi bits
    chars.setRange(127, 255);

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    std::array<u8, 128> t1;
    t1.fill('*'); // it's full of stars!

    for (unsigned int c = 127; c <= 255; c++) {
        t1[40] = (u8)c;
        const u8 *rv = truffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1.data()), reinterpret_cast<u8 *>(t1.data()) + 128);

        ASSERT_EQ((size_t)t1.data() + 40, (size_t)rv);
    }
}

TEST(ReverseTruffle, ExecNoMatch1) {
    m128 mask1, mask2;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    char t[] = " bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char *t1 = t + 1;
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t), rv);
    }
}

TEST(ReverseTruffle, ExecNoMatch2) {
    m128 mask1, mask2;

    CharReach chars;

    chars.set('a');
    chars.set('B');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    char t[] = " bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char *t1 = t + 1;
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t), rv);
    }
}

TEST(ReverseTruffle, ExecNoMatch3) {
    m128 mask1, mask2;

    CharReach chars;
    chars.set('V'); /* V = 0x56, e = 0x65 */

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    char t[] = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
    char *t1 = t + 1;
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t), rv);
    }
}

TEST(ReverseTruffle, ExecMiniMatch0) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    char t1[] = "a";

    const u8 *rv = rtruffleExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + strlen(t1));

    ASSERT_EQ((size_t)t1, (size_t)rv);
}

TEST(ReverseTruffle, ExecMiniMatch1) {
    m128 mask1, mask2;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbabbbb";
    size_t len = strlen(t1);

    const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len);
    ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
    EXPECT_EQ('a', (char)*rv);
    ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 7, rv);
}

TEST(ReverseTruffle, ExecMiniMatch2) {
    m128 mask1, mask2;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "babbbbbabbbb";
    size_t len = strlen(t1);

    const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len);
    ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
    EXPECT_EQ('a', (char)*rv);
    ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 7, rv);
}


TEST(ReverseTruffle, ExecMatch1) {
    m128 mask1, mask2;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbabbbbbbbbbbabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 17, rv);
    }
}

TEST(ReverseTruffle, ExecMatch2) {
    m128 mask1, mask2;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbabbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 32, rv);
    }
}

TEST(ReverseTruffle, ExecMatch3) {
    m128 mask1, mask2;

    CharReach chars;
    chars.set('a');
    chars.set('B');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaBbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
        EXPECT_EQ('B', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 32, rv);
    }

    // check that we match the 'a' bytes as well.
    ASSERT_EQ('B', t1[32]);
    t1[32] = 'b';
    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 31, rv);
    }
}

TEST(ReverseTruffle, ExecMatch4) {
    m128 mask1, mask2;

    CharReach chars;
    chars.set('a');
    chars.set('C');
    chars.set('A');
    chars.set('c');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaAbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t2[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaCbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t3[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaacbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t4[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        EXPECT_EQ('A', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 32, rv);

        rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t2), reinterpret_cast<u8 *>(t2) + len - i);
        EXPECT_EQ('C', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t2) + 32, rv);

        rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t3), reinterpret_cast<u8 *>(t3) + len - i);
        EXPECT_EQ('c', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t3) + 32, rv);

        rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t4), reinterpret_cast<u8 *>(t4) + len - i);
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t4) + 32, rv);
    }
}

TEST(ReverseTruffle, ExecMatch5) {
    m128 mask1, mask2;

    CharReach chars;
    chars.set('a');

    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < len; i++) {
        t1[i] = 'a';
        const u8 *rv = rtruffleExec(mask1, mask2, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + i, rv);
    }
}

/*
 * Additional unit tests for truffle accelerator.
 * These cover edge cases and areas not handled by the original test suite.
 */

// --- Compile/Roundtrip Tests ---

TEST(Truffle, CompileRanges) {
    // Test building masks for various character ranges and verify roundtrip
    m128 mask1, mask2;
    CharReach chars;

    // Printable ASCII range
    chars.setRange(0x20, 0x7e);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    CharReach out = truffle2cr(reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    ASSERT_EQ(out, chars);

    // High byte range
    chars.clear();
    chars.setRange(0x80, 0xff);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    out = truffle2cr(reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    ASSERT_EQ(out, chars);

    // Mixed low and high range
    chars.clear();
    chars.setRange(0x30, 0x39); // digits
    chars.setRange(0xC0, 0xDF); // some high bytes
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    out = truffle2cr(reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    ASSERT_EQ(out, chars);
}

TEST(Truffle, CompileEmpty) {
    // Empty character class - no characters set
    m128 mask1, mask2;
    CharReach chars; // empty
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    CharReach out = truffle2cr(reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    ASSERT_EQ(out, chars);
    ASSERT_TRUE(out.none());
}

TEST(Truffle, CompileSameNibbleDiffHigh) {
    // Characters with the same low nibble but different high nibble
    // e.g., 'V' = 0x56 and 'e' = 0x65 have different nibbles, but
    // 0x13 and 0x23 share low nibble 0x3 but differ in bits 4-6
    m128 mask1, mask2;
    CharReach chars;
    chars.set(0x13);
    chars.set(0x23);
    chars.set(0x43);
    chars.set(0x93);
    chars.set(0xA3);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    CharReach out = truffle2cr(reinterpret_cast<u8 *>(&mask1), reinterpret_cast<u8 *>(&mask2));
    ASSERT_EQ(out, chars);
}

// --- Forward Exec: Edge cases ---

TEST(Truffle, ExecSingleByte) {
    // Buffer of length 1 - matching char
    m128 lo, hi;
    CharReach chars;
    chars.set('Z');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[1] = { 'Z' };
    const u8 *rv = truffleExec(lo, hi, buf, buf + 1);
    ASSERT_EQ(buf, rv);
}

TEST(Truffle, ExecSingleByteNoMatch) {
    // Buffer of length 1 - non-matching char
    m128 lo, hi;
    CharReach chars;
    chars.set('Z');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[1] = { 'A' };
    const u8 *rv = truffleExec(lo, hi, buf, buf + 1);
    ASSERT_EQ(buf + 1, rv);
}

TEST(Truffle, ExecHighByteMatch) {
    // Test high byte characters (>= 0x80) match correctly
    m128 lo, hi;
    CharReach chars;
    chars.set(0xAB);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[128];
    memset(buf, 0x20, sizeof(buf));

    for (size_t pos = 0; pos < 64; pos++) {
        buf[pos] = 0xAB;
        const u8 *rv = truffleExec(lo, hi, buf, buf + 128);
        ASSERT_EQ(buf + pos, rv);
        buf[pos] = 0x20; // restore
    }
}

TEST(Truffle, ExecHighByteNoMatchSameNibble) {
    // 0xAB and 0x2B share the same low nibble (0xB).
    // Searching for 0xAB should NOT match 0x2B.
    m128 lo, hi;
    CharReach chars;
    chars.set(0xAB);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[64];
    memset(buf, 0x2B, sizeof(buf)); // same low nibble, different high nibble
    const u8 *rv = truffleExec(lo, hi, buf, buf + 64);
    ASSERT_EQ(buf + 64, rv);
}

TEST(Truffle, ExecNulCharMatch) {
    // Searching for NUL character
    m128 lo, hi;
    CharReach chars;
    chars.set(0x00);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[80];
    memset(buf, 0xFF, sizeof(buf));
    buf[45] = 0x00;
    const u8 *rv = truffleExec(lo, hi, buf, buf + 80);
    ASSERT_EQ(buf + 45, rv);
}

TEST(Truffle, ExecDotMatchAll) {
    // Dot (all chars) should match the first byte
    m128 lo, hi;
    CharReach chars;
    chars.setall();
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[64];
    memset(buf, 0x42, sizeof(buf));
    const u8 *rv = truffleExec(lo, hi, buf, buf + 64);
    ASSERT_EQ(buf, rv); // first byte always matches
}

TEST(Truffle, ExecMatchAtBufferEnd) {
    // Match only at the very last byte of the buffer
    m128 lo, hi;
    CharReach chars;
    chars.set('X');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[200];
    memset(buf, '.', sizeof(buf));
    buf[199] = 'X';
    const u8 *rv = truffleExec(lo, hi, buf, buf + 200);
    ASSERT_EQ(buf + 199, rv);
}

TEST(Truffle, ExecVaryingLengths) {
    // Test with buffers of many different lengths (1 to 130)
    m128 lo, hi;
    CharReach chars;
    chars.set('q');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[130];
    for (size_t len = 1; len <= 130; len++) {
        memset(buf, '.', len);
        // No match
        const u8 *rv = truffleExec(lo, hi, buf, buf + len);
        ASSERT_EQ(buf + len, rv) << "len=" << len;

        // Match at last position
        buf[len - 1] = 'q';
        rv = truffleExec(lo, hi, buf, buf + len);
        ASSERT_EQ(buf + len - 1, rv) << "len=" << len;
    }
}

TEST(Truffle, ExecAlignmentSweep) {
    // Test match at every offset within a larger buffer to exercise
    // alignment code paths
    m128 lo, hi;
    CharReach chars;
    chars.set('!');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[256];
    memset(buf, '.', sizeof(buf));

    for (size_t offset = 0; offset < 64; offset++) {
        buf[offset] = '!';
        const u8 *rv = truffleExec(lo, hi, buf, buf + 256);
        ASSERT_EQ(buf + offset, rv) << "offset=" << offset;
        buf[offset] = '.';
    }
}

TEST(Truffle, ExecMultipleCharsInClass) {
    // Test character class with many different characters
    m128 lo, hi;
    CharReach chars;
    // [a-zA-Z0-9]
    chars.setRange('a', 'z');
    chars.setRange('A', 'Z');
    chars.setRange('0', '9');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    // Buffer of non-matching chars
    u8 buf[64];
    memset(buf, '!', sizeof(buf)); // not in character class

    for (u8 c = '0'; c <= '9'; c++) {
        buf[32] = c;
        const u8 *rv = truffleExec(lo, hi, buf, buf + 64);
        ASSERT_EQ(buf + 32, rv) << "char=" << (char)c;
        buf[32] = '!';
    }
    for (u8 c = 'a'; c <= 'z'; c++) {
        buf[32] = c;
        const u8 *rv = truffleExec(lo, hi, buf, buf + 64);
        ASSERT_EQ(buf + 32, rv) << "char=" << (char)c;
        buf[32] = '!';
    }
    for (u8 c = 'A'; c <= 'Z'; c++) {
        buf[32] = c;
        const u8 *rv = truffleExec(lo, hi, buf, buf + 64);
        ASSERT_EQ(buf + 32, rv) << "char=" << (char)c;
        buf[32] = '!';
    }
}

TEST(Truffle, ExecAllSingleCharClasses) {
    // For every possible character class of size 1, verify match works
    for (unsigned c = 0; c < 256; c++) {
        m128 lo, hi;
        CharReach chars;
        chars.set((u8)c);
        truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

        // Build a buffer that doesn't contain c (use c^1 if possible)
        u8 filler = (u8)(c ^ 0x01);
        if (filler == (u8)c) filler = (u8)(c ^ 0x02);
        u8 buf[64];
        memset(buf, filler, sizeof(buf));

        // No match
        const u8 *rv = truffleExec(lo, hi, buf, buf + 64);
        ASSERT_EQ(buf + 64, rv) << "c=" << c;

        // Place character at position 17
        buf[17] = (u8)c;
        rv = truffleExec(lo, hi, buf, buf + 64);
        ASSERT_EQ(buf + 17, rv) << "c=" << c;
    }
}

// --- Reverse Exec: Edge cases ---

TEST(ReverseTruffle, ExecSingleByte) {
    m128 lo, hi;
    CharReach chars;
    chars.set('Z');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[1] = { 'Z' };
    const u8 *rv = rtruffleExec(lo, hi, buf, buf + 1);
    ASSERT_EQ(buf, rv);
}

TEST(ReverseTruffle, ExecSingleByteNoMatch) {
    m128 lo, hi;
    CharReach chars;
    chars.set('Z');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 raw[2] = { 0, 'A' };
    const u8 *buf = raw + 1;
    const u8 *rv = rtruffleExec(lo, hi, buf, buf + 1);
    ASSERT_EQ(raw, rv);
}

TEST(ReverseTruffle, ExecHighByteReverse) {
    // Reverse search for high byte characters
    m128 lo, hi;
    CharReach chars;
    chars.set(0xFE);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[128];
    memset(buf, 0x20, sizeof(buf));

    // Place match at various positions and verify reverse finds the last one
    for (size_t pos = 64; pos < 128; pos++) {
        memset(buf, 0x20, sizeof(buf));
        buf[pos] = 0xFE;
        const u8 *rv = rtruffleExec(lo, hi, buf, buf + 128);
        ASSERT_EQ(buf + pos, rv) << "pos=" << pos;
    }
}

TEST(ReverseTruffle, ExecNulReverse) {
    // Reverse search for NUL
    m128 lo, hi;
    CharReach chars;
    chars.set(0x00);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[80];
    memset(buf, 0xFF, sizeof(buf));
    buf[10] = 0x00;
    buf[50] = 0x00;
    const u8 *rv = rtruffleExec(lo, hi, buf, buf + 80);
    ASSERT_EQ(buf + 50, rv); // should find the last NUL
}

TEST(ReverseTruffle, ExecMatchAtBufferStart) {
    // Match only at the very first byte
    m128 lo, hi;
    CharReach chars;
    chars.set('X');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[200];
    memset(buf, '.', sizeof(buf));
    buf[0] = 'X';
    const u8 *rv = rtruffleExec(lo, hi, buf, buf + 200);
    ASSERT_EQ(buf, rv);
}

TEST(ReverseTruffle, ExecVaryingLengths) {
    // Test reverse with buffers of many different lengths
    m128 lo, hi;
    CharReach chars;
    chars.set('q');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 raw[131];
    u8 *buf = raw + 1;
    for (size_t len = 1; len <= 130; len++) {
        memset(buf, '.', len);
        // No match
        const u8 *rv = rtruffleExec(lo, hi, buf, buf + len);
        ASSERT_EQ(raw, rv) << "len=" << len;

        // Match at first position
        buf[0] = 'q';
        rv = rtruffleExec(lo, hi, buf, buf + len);
        ASSERT_EQ(buf, rv) << "len=" << len;
    }
}

TEST(ReverseTruffle, ExecLargeBuffer) {
    // Large buffer reverse test
    m128 lo, hi;
    CharReach chars;
    chars.set('!');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[4096];
    memset(buf, '.', sizeof(buf));
    buf[2048] = '!';

    const u8 *rv = rtruffleExec(lo, hi, buf, buf + 4096);
    ASSERT_EQ(buf + 2048, rv);
}

TEST(ReverseTruffle, ExecAlignmentSweep) {
    // Reverse alignment sweep
    m128 lo, hi;
    CharReach chars;
    chars.set('#');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[256];
    memset(buf, '.', sizeof(buf));

    for (size_t offset = 192; offset < 256; offset++) {
        buf[offset] = '#';
        const u8 *rv = rtruffleExec(lo, hi, buf, buf + 256);
        ASSERT_EQ(buf + offset, rv) << "offset=" << offset;
        buf[offset] = '.';
    }
}

TEST(ReverseTruffle, ExecAllSingleCharClasses) {
    // For every possible single-char class, verify reverse match works
    for (unsigned c = 0; c < 256; c++) {
        m128 lo, hi;
        CharReach chars;
        chars.set((u8)c);
        truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

        u8 filler = (u8)(c ^ 0x01);
        if (filler == (u8)c) filler = (u8)(c ^ 0x02);
        u8 raw[65];
        u8 *buf = raw + 1;
        memset(buf, filler, 64);

        // No match
        const u8 *rv = rtruffleExec(lo, hi, buf, buf + 64);
        ASSERT_EQ(raw, rv) << "c=" << c;

        // Place character at position 40
        buf[40] = (u8)c;
        rv = rtruffleExec(lo, hi, buf, buf + 64);
        ASSERT_EQ(buf + 40, rv) << "c=" << c;
    }
}

TEST(ReverseTruffle, ExecMultipleMatches) {
    // Verify reverse finds the LAST match
    m128 lo, hi;
    CharReach chars;
    chars.set('a');
    chars.set('b');
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[128];
    memset(buf, '.', sizeof(buf));
    buf[10] = 'a';
    buf[50] = 'b';
    buf[90] = 'a';

    const u8 *rv = rtruffleExec(lo, hi, buf, buf + 128);
    ASSERT_EQ(buf + 90, rv);

    // Now check with restricted end
    rv = rtruffleExec(lo, hi, buf, buf + 60);
    ASSERT_EQ(buf + 50, rv);
}

// --- Forward: Boundary between low and high characters ---

TEST(Truffle, ExecBoundaryChars) {
    // Test characters at the 0x7f/0x80 boundary
    m128 lo, hi;
    CharReach chars;
    chars.set(0x7f);
    chars.set(0x80);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[64];
    memset(buf, 0x41, sizeof(buf));

    buf[20] = 0x80;
    const u8 *rv = truffleExec(lo, hi, buf, buf + 64);
    ASSERT_EQ(buf + 20, rv);

    buf[20] = 0x41;
    buf[15] = 0x7f;
    rv = truffleExec(lo, hi, buf, buf + 64);
    ASSERT_EQ(buf + 15, rv);
}

TEST(ReverseTruffle, ExecBoundaryChars) {
    m128 lo, hi;
    CharReach chars;
    chars.set(0x7f);
    chars.set(0x80);
    truffleBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));

    u8 buf[64];
    memset(buf, 0x41, sizeof(buf));

    buf[40] = 0x7f;
    buf[50] = 0x80;
    const u8 *rv = rtruffleExec(lo, hi, buf, buf + 64);
    ASSERT_EQ(buf + 50, rv);
}
