/*
 * Copyright (c) 2015-2017, Intel Corporation
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

#include "config.h"

#include <set>
#include <sys/mman.h>

#include "gtest/gtest.h"
#include "nfa/shufti.h"
#include "nfa/shufticompile.h"
#include "util/target_info.h"

using namespace ue2;
using std::set;
using std::pair;
using std::make_pair;

TEST(Shufti, BuildMask1) {
    m128 lomask, himask;

    CharReach chars;

    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lomask), reinterpret_cast<u8 *>(&himask));
    ASSERT_NE(-1, ret);

    u8 *lo = reinterpret_cast<u8 *>(&lomask);
    u8 *hi = reinterpret_cast<u8 *>(&himask);
    for (int i = 0; i < 16; i++) {
        if (i == 'a' % 16) {
            ASSERT_EQ(1, lo[i]);
        } else {
            ASSERT_EQ(0, lo[i]);
        }

        if (i == 'a' >> 4) {
            ASSERT_EQ(1, hi[i]);
        } else {
            ASSERT_EQ(0, hi[i]);
        }
    }
}

TEST(Shufti, BuildMask2) {
    m128 lomask, himask;

    CharReach chars;

    chars.set('a');
    chars.set('B');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lomask), reinterpret_cast<u8 *>(&himask));
    ASSERT_NE(-1, ret);

    const u8 *lo = reinterpret_cast<u8 *>(&lomask);
    const u8 *hi = reinterpret_cast<u8 *>(&himask);
    ASSERT_TRUE(lo['a' % 16] & hi['a' >> 4]);
    ASSERT_TRUE(lo['B' % 16] & hi['B' >> 4]);
    ASSERT_FALSE(lo['a' % 16] & hi['B' >> 4]);
    ASSERT_FALSE(lo['B' % 16] & hi['a' >> 4]);
 }

TEST(Shufti, BuildMask4) {
    m128 lomask, himask;

    CharReach chars;

    chars.set('a');
    chars.set('B');
    chars.set('A');
    chars.set('b');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lomask), reinterpret_cast<u8 *>(&himask));
    ASSERT_NE(-1, ret);

    const u8 *lo = reinterpret_cast<u8 *>(&lomask);
    const u8 *hi = reinterpret_cast<u8 *>(&himask);
    ASSERT_TRUE(lo['a' % 16] & hi['a' >> 4]);
    ASSERT_TRUE(lo['A' % 16] & hi['A' >> 4]);
    ASSERT_TRUE(lo['b' % 16] & hi['b' >> 4]);
    ASSERT_TRUE(lo['B' % 16] & hi['B' >> 4]);
}

TEST(Shufti, ExecNoMatch1) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 32; i++) {
        const u8 *rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_LE(((size_t)t1 + strlen(t1)) & ~0xf, (size_t)rv);
    }
}

TEST(Shufti, ExecNoMatch2) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');
    chars.set('B');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_LE(((size_t)t1 + strlen(t1)) & ~0xf, (size_t)rv);
    }
}

TEST(Shufti, ExecNoMatch3) {
    m128 lo, hi;

    CharReach chars;
    chars.set('V'); /* V = 0x56, e = 0x65 */

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    char t1[] = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_LE(((size_t)t1 + strlen(t1)) & ~0xf, (size_t)rv);
    }
}

TEST(Shufti, ExecMatch1) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbabbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 32; i++) {
        const u8 *rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 33, (size_t)rv);
    }
}

TEST(Shufti, ExecMatch2) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(Shufti, ExecMatch3) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');
    chars.set('B');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbBaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(Shufti, ExecMatch4) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');
    chars.set('C');
    chars.set('A');
    chars.set('c');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbAaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t2[] = "bbbbbbbbbbbbbbbbbCaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t3[] = "bbbbbbbbbbbbbbbbbcaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t4[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);

        rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t2) + i, reinterpret_cast<u8 *>(t2) + strlen(t1));

        ASSERT_EQ((size_t)t2 + 17, (size_t)rv);

        rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t3) + i, reinterpret_cast<u8 *>(t3) + strlen(t3));

        ASSERT_EQ((size_t)t3 + 17, (size_t)rv);

        rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t4) + i, reinterpret_cast<u8 *>(t4) + strlen(t4));

        ASSERT_EQ((size_t)t4 + 17, (size_t)rv);
    }
}

TEST(Shufti, ExecMatch5) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 31; i++) {
        t1[48 - i] = 'a';
        const u8 *rv = shuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)&t1[48 - i], (size_t)rv);
    }
}

TEST(DoubleShufti, BuildMask1) {
    m128 lo1m, hi1m, lo2m, hi2m;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a', 'B'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1m), reinterpret_cast<u8 *>(&hi1m),
                                     reinterpret_cast<u8 *>(&lo2m), reinterpret_cast<u8 *>(&hi2m));
    ASSERT_TRUE(ret);

    u8 *lo1 = reinterpret_cast<u8 *>(&lo1m);
    u8 *lo2 = reinterpret_cast<u8 *>(&lo2m);
    u8 *hi1 = reinterpret_cast<u8 *>(&hi1m);
    u8 *hi2 = reinterpret_cast<u8 *>(&hi2m);
    for (int i = 0; i < 16; i++) {
        if (i == 'a' % 16) {
            ASSERT_EQ(254, lo1[i]);
        } else {
            ASSERT_EQ(255, lo1[i]);
        }

        if (i == 'a' >> 4) {
            ASSERT_EQ(254, hi1[i]);
        } else {
            ASSERT_EQ(255, hi1[i]);
        }

        if (i == 'B' % 16) {
            ASSERT_EQ(254, lo2[i]);
        } else {
            ASSERT_EQ(255, lo2[i]);
        }

        if (i == 'B' >> 4) {
            ASSERT_EQ(254, hi2[i]);
        } else {
            ASSERT_EQ(255, hi2[i]);
        }
    }
}

TEST(DoubleShufti, BuildMask2) {
    m128 lo1m, hi1m, lo2m, hi2m;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','z'));
    lits.insert(make_pair('B','z'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1m), reinterpret_cast<u8 *>(&hi1m),
                                     reinterpret_cast<u8 *>(&lo2m), reinterpret_cast<u8 *>(&hi2m));
    ASSERT_TRUE(ret);

    const u8 *lo1 = reinterpret_cast<u8 *>(&lo1m);
    const u8 *lo2 = reinterpret_cast<u8 *>(&lo2m);
    const u8 *hi1 = reinterpret_cast<u8 *>(&hi1m);
    const u8 *hi2 = reinterpret_cast<u8 *>(&hi2m);
    ASSERT_NE(0xff,
              lo1['a' % 16] | hi1['a' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_NE(0xff,
              lo1['B' % 16] | hi1['B' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_EQ(0xff,
              lo1['a' % 16] | hi1['B' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_EQ(0xff,
              lo1['B' % 16] | hi1['a' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
}

TEST(DoubleShufti, BuildMask4) {
    m128 lo1m, hi1m, lo2m, hi2m;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','z'));
    lits.insert(make_pair('B','z'));
    lits.insert(make_pair('A','z'));
    lits.insert(make_pair('b','z'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1m), reinterpret_cast<u8 *>(&hi1m),
                                     reinterpret_cast<u8 *>(&lo2m), reinterpret_cast<u8 *>(&hi2m));
    ASSERT_TRUE(ret);

    const u8 *lo1 = reinterpret_cast<u8 *>(&lo1m);
    const u8 *lo2 = reinterpret_cast<u8 *>(&lo2m);
    const u8 *hi1 = reinterpret_cast<u8 *>(&hi1m);
    const u8 *hi2 = reinterpret_cast<u8 *>(&hi2m);
    ASSERT_NE(0xff,
              lo1['a' % 16] | hi1['a' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_NE(0xff,
              lo1['A' % 16] | hi1['A' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_NE(0xff,
              lo1['b' % 16] | hi1['b' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_NE(0xff,
              lo1['B' % 16] | hi1['B' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
}

TEST(DoubleShufti, BuildMask5) {
    m128 lo1m, hi1m;
    m128 lo2m, hi2m;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','z'));

    CharReach bytes;
    bytes.set('X');

    bool ret = shuftiBuildDoubleMasks(bytes, lits, reinterpret_cast<u8 *>(&lo1m), reinterpret_cast<u8 *>(&hi1m),
                                     reinterpret_cast<u8 *>(&lo2m), reinterpret_cast<u8 *>(&hi2m));
    ASSERT_TRUE(ret);

    const u8 *lo1 = reinterpret_cast<u8 *>(&lo1m);
    const u8 *lo2 = reinterpret_cast<u8 *>(&lo2m);
    const u8 *hi1 = reinterpret_cast<u8 *>(&hi1m);
    const u8 *hi2 = reinterpret_cast<u8 *>(&hi2m);
    ASSERT_NE(0xff,
              lo1['a' % 16] | hi1['a' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_EQ(0xff,
              lo1['a' % 16] | hi1['a' >> 4] | lo2['X' % 16] | hi2['X' >> 4]);
    ASSERT_EQ(0xff,
              lo1['A' % 16] | hi1['A' >> 4] | lo2['X' % 16] | hi2['X' >> 4]);
    ASSERT_EQ(0xff,
              lo1['b' % 16] | hi1['b' >> 4] | lo2['X' % 16] | hi2['X' >> 4]);
    ASSERT_EQ(0xff,
              lo1['B' % 16] | hi1['B' >> 4] | lo2['X' % 16] | hi2['X' >> 4]);
}

TEST(DoubleShufti, BuildMask6) {
    m128 lo1m, hi1m, lo2m, hi2m;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','z'));
    lits.insert(make_pair('B','z'));
    lits.insert(make_pair('A','z'));
    lits.insert(make_pair('b','z'));
    lits.insert(make_pair('a','y'));
    lits.insert(make_pair('B','y'));
    lits.insert(make_pair('A','y'));
    lits.insert(make_pair('b','y'));
    lits.insert(make_pair('a','x'));
    lits.insert(make_pair('B','x'));
    lits.insert(make_pair('A','x'));
    lits.insert(make_pair('b','x'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1m), reinterpret_cast<u8 *>(&hi1m),
                                     reinterpret_cast<u8 *>(&lo2m), reinterpret_cast<u8 *>(&hi2m));
    ASSERT_TRUE(ret);

    const u8 *lo1 = reinterpret_cast<u8 *>(&lo1m);
    const u8 *lo2 = reinterpret_cast<u8 *>(&lo2m);
    const u8 *hi1 = reinterpret_cast<u8 *>(&hi1m);
    const u8 *hi2 = reinterpret_cast<u8 *>(&hi2m);
    ASSERT_NE(0xff,
              lo1['a' % 16] | hi1['a' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_NE(0xff,
              lo1['A' % 16] | hi1['A' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_NE(0xff,
              lo1['b' % 16] | hi1['b' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_NE(0xff,
              lo1['B' % 16] | hi1['B' >> 4] | lo2['z' % 16] | hi2['z' >> 4]);
    ASSERT_NE(0xff,
              lo1['a' % 16] | hi1['a' >> 4] | lo2['y' % 16] | hi2['y' >> 4]);
    ASSERT_NE(0xff,
              lo1['A' % 16] | hi1['A' >> 4] | lo2['y' % 16] | hi2['y' >> 4]);
    ASSERT_NE(0xff,
              lo1['b' % 16] | hi1['b' >> 4] | lo2['y' % 16] | hi2['y' >> 4]);
    ASSERT_NE(0xff,
              lo1['B' % 16] | hi1['B' >> 4] | lo2['y' % 16] | hi2['y' >> 4]);
    ASSERT_NE(0xff,
              lo1['a' % 16] | hi1['a' >> 4] | lo2['x' % 16] | hi2['x' >> 4]);
    ASSERT_NE(0xff,
              lo1['A' % 16] | hi1['A' >> 4] | lo2['x' % 16] | hi2['x' >> 4]);
    ASSERT_NE(0xff,
              lo1['b' % 16] | hi1['b' >> 4] | lo2['x' % 16] | hi2['x' >> 4]);
    ASSERT_NE(0xff,
              lo1['B' % 16] | hi1['B' >> 4] | lo2['x' % 16] | hi2['x' >> 4]);
}

TEST(DoubleShufti, BuildMask7) {
    m128 lo1m, hi1m, lo2m, hi2m;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','b'));
    lits.insert(make_pair('c','d'));
    lits.insert(make_pair('e','f'));
    lits.insert(make_pair('g','h'));
    lits.insert(make_pair('i','j'));
    lits.insert(make_pair('k','l'));
    lits.insert(make_pair('m','n'));
    lits.insert(make_pair('o','p'));
    lits.insert(make_pair('q','r'));
    lits.insert(make_pair('s','t'));
    lits.insert(make_pair('u','v'));
    lits.insert(make_pair('w','x'));

    bool rv = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1m), reinterpret_cast<u8 *>(&hi1m),
                                     reinterpret_cast<u8 *>(&lo2m), reinterpret_cast<u8 *>(&hi2m));
    ASSERT_FALSE(rv);
}

TEST(DoubleShufti, ExecNoMatch1) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','b'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                     reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_LE(((size_t)t1 + strlen(t1)) & ~0xf, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecNoMatch1b) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('b','a'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_LE((size_t)t1 + i + 15, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecNoMatch2) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','b'));
    lits.insert(make_pair('B','b'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_LE(((size_t)t1 + strlen(t1)) & ~0xf, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecNoMatch2b) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('b','a'));
    lits.insert(make_pair('b','B'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2, reinterpret_cast<u8 *>(t1) + i,
                                        reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_LE((size_t)t1 + i + 15, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecNoMatch3) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('V','e'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    char t1[] = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_LE(((size_t)t1 + strlen(t1)) & ~0xf, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecNoMatch3b) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('e','V'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    char t1[] = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_LE((size_t)t1 + i + 15, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatchShort1) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','b'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbabbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatch1) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','b'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatch2) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','a'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatch3) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('B','a'));
    lits.insert(make_pair('a','a'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbBaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatch4) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('A','a'));
    lits.insert(make_pair('a','a'));
    lits.insert(make_pair('C','a'));
    lits.insert(make_pair('c','a'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbAaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t2[] = "bbbbbbbbbbbbbbbbbCaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t3[] = "bbbbbbbbbbbbbbbbbcaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t4[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);

        rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                              reinterpret_cast<u8 *>(t2) + i, reinterpret_cast<u8 *>(t2) + strlen(t2));

        ASSERT_EQ((size_t)t2 + 17, (size_t)rv);

        rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                              reinterpret_cast<u8 *>(t3)+ i, reinterpret_cast<u8 *>(t3) + strlen(t3));

        ASSERT_EQ((size_t)t3 + 17, (size_t)rv);

        rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                              reinterpret_cast<u8 *>(t4) + i, reinterpret_cast<u8 *>(t4) + strlen(t4));

        ASSERT_EQ((size_t)t4 + 17, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatch4b) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','A'));
    lits.insert(make_pair('a','a'));
    lits.insert(make_pair('a','C'));
    lits.insert(make_pair('a','c'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaAaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t2[] = "bbbbbbbbbbbbbbbbbaCaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t3[] = "bbbbbbbbbbbbbbbbbacaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";
    char t4[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbabbbbbbbbbbbb";

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1) + i, reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)t1 + 17, (size_t)rv);

        rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                              reinterpret_cast<u8 *>(t2) + i, reinterpret_cast<u8 *>(t2) + strlen(t2));

        ASSERT_EQ((size_t)t2 + 17, (size_t)rv);

        rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                              reinterpret_cast<u8 *>(t3)+ i, reinterpret_cast<u8 *>(t3) + strlen(t3));

        ASSERT_EQ((size_t)t3 + 17, (size_t)rv);

        rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                              reinterpret_cast<u8 *>(t4) + i, reinterpret_cast<u8 *>(t4) + strlen(t4));

        ASSERT_EQ((size_t)t4 + 17, (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatch5) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','A'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 31; i++) {
        t1[48 - i] = 'a';
        t1[48 - i + 1] = 'A';
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)&t1[48 - i], (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatchMixed1) {
    m128 lo1, hi1, lo2, hi2;

    CharReach onebyte;
    flat_set<pair<u8, u8>> twobyte;

    // just one one-byte literal
    onebyte.set('a');

    bool ret = shuftiBuildDoubleMasks(onebyte, twobyte, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 31; i++) {
        t1[48 - i] = 'a';
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)&t1[48 - i], (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatchMixed2) {
    m128 lo1, hi1, lo2, hi2;

    CharReach onebyte;
    flat_set<pair<u8, u8>> twobyte;

    // one of each
    onebyte.set('a');
    twobyte.insert(make_pair('x', 'y'));

    bool ret = shuftiBuildDoubleMasks(onebyte, twobyte, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t2[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";

    for (size_t i = 0; i < 31; i++) {
        t1[48 - i] = 'a';
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + strlen(t1));

        ASSERT_EQ((size_t)&t1[48 - i], (size_t)rv);
    }

    for (size_t i = 0; i < 31; i++) {
        t2[48 - i] = 'x';
        t2[48 - i + 1] = 'y';
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t2), reinterpret_cast<u8 *>(t2) + strlen(t1));

        ASSERT_EQ((size_t)&t2[48 - i], (size_t)rv);
    }
}

TEST(DoubleShufti, ExecMatchMixed3) {
    m128 lo1, hi1, lo2, hi2;

    CharReach onebyte;
    flat_set<pair<u8, u8>> twobyte;

    // one of each
    onebyte.set('a');
    twobyte.insert(make_pair('x', 'y'));

    bool ret = shuftiBuildDoubleMasks(onebyte, twobyte, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const int len = 420;
    char t1[len + 1];
    char t2[len + 2];
    memset(t1, 'b', len);
    memset(t2, 'b', len);

    for (size_t i = 0; i < 400; i++) {
        t1[len - i] = 'a';
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ((size_t)&t1[len - i], (size_t)rv);
    }

    for (size_t i = 0; i < 400; i++) {
        t2[len - i] = 'x';
        t2[len - i + 1] = 'y';
        DEBUG_PRINTF("i = %ld\n", i);
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t2), reinterpret_cast<u8 *>(t2) + len);

        // Same form as the single-char loop above: i==0 is buf_end, i==1 a resume
        // point (first char on the last byte), i>=2 the real match.
        ASSERT_EQ(reinterpret_cast<const u8 *>(&t2[len - i]), rv);
    }
}

// Double shufti used to report matches when the first char of a pair landed at
// the end of a vector. This test check for the regression.
TEST(DoubleShufti, ExecNoMatchVectorEdge) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    lits.insert(make_pair('a','c'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const int len = 80;
    char t1[len + 2];
    memset(t1, 'b', len);

    for (size_t i = 0; i < 70; i++) {
        t1[len - i] = 'a';
        t1[len - i + 1] = 'b';
        DEBUG_PRINTF("i = %ld\n", i);
        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len);

        // i==1: first char 'a' is the last byte, so accel stops at buf_end-1
        ASSERT_EQ(i == 1 ? reinterpret_cast<const u8 *>(t1 + len - 1) : reinterpret_cast<const u8 *>(t1 + len), rv);
    }
}

// Regression test: shuftiDoubleExecReal used to read a full vector (S bytes)
// in the tail, which could overread past buf_end. If the buffer ends at a page
// boundary followed by an unmapped page, this causes a SIGSEGV.
TEST(DoubleShufti, ExecNoOverreadPageBoundary) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;
    lits.insert(make_pair('a','b'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits,
                                      reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const size_t page_size = sysconf(_SC_PAGE_SIZE);
    // Map two pages, then unmap the second to create a guard page.
    u8 *pages = reinterpret_cast<u8 *>(mmap(nullptr, 2 * page_size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
    ASSERT_NE(MAP_FAILED, reinterpret_cast<void *>(pages));
    ASSERT_EQ(0, munmap(pages + page_size, page_size));

    u8 *page_end = pages + page_size;

    // Test various buffer lengths ending right at the page boundary.
    // Without the fix, lengths that are not a multiple of the vector size will
    // cause the tail code to read past page_end into the unmapped guard page.
    for (size_t len = 1; len <= 256; len++) {
        u8 *buf = page_end - len;
        memset(buf, 'c', len); // fill with non-matching data

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2, buf, page_end);
        // No match expected: rv should be at or past buf_end
        ASSERT_GE((size_t)rv, (size_t)(page_end - 1))
            << "Unexpected match at len=" << len;
    }

    // Also test with an actual match near the end.
    for (size_t len = 2; len <= 256; len++) {
        u8 *buf = page_end - len;
        memset(buf, 'c', len);
        // Place matching pair "ab" near the end of the buffer.
        buf[len - 2] = 'a';
        buf[len - 1] = 'b';

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2, buf, page_end);
        ASSERT_EQ((size_t)(page_end - 2), (size_t)rv)
            << "Match not found at expected position for len=" << len;
    }

    munmap(pages, page_size);
}

TEST(DoubleShufti, ExecMatchVectorEdge) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;
    lits.insert(make_pair('a', 'b'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const int len = 80;
    const int vector_size = 16;
    for (size_t start = 1; start < vector_size; start++) {
        char t[len];
        memset(t, 'z', len);
        char *buf = t + start;
        int buf_len = len - start;

        uintptr_t aligned = (reinterpret_cast<uintptr_t>(buf) + vector_size - 1) &
                            ~static_cast<uintptr_t>(vector_size - 1);
        ptrdiff_t boundary = reinterpret_cast<const char *>(aligned) - buf;

        if (boundary < 1 || boundary >= buf_len - 1) continue;

        buf[boundary - 1] = 'a';
        buf[boundary] = 'b';

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(buf),
                                        reinterpret_cast<u8 *>(buf) + buf_len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(buf + boundary - 1), rv)
            << "Failed for start=" << start << " boundary=" << boundary;
    }
}

TEST(DoubleShufti, ExecNoMatchLastByte) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;
    lits.insert(make_pair('x', 'y'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const int maxlen = 80;
    char t1[maxlen + 1];
    for (int len = 17; len < maxlen; len++) {
        memset(t1, 'b', len + 1);
        t1[len - 1] = 'x';

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1),
                                        reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1 + len - 1), rv)
            << "Failed for len=" << len;
    }
}

TEST(DoubleShufti, ExecMatchLastByte) {
    m128 lo1, hi1, lo2, hi2;

    CharReach onebyte;
    flat_set<pair<u8, u8>> twobyte;

    onebyte.set('a');
    twobyte.insert(make_pair('x', 'y'));

    bool ret = shuftiBuildDoubleMasks(onebyte, twobyte, reinterpret_cast<u8 *>(&lo1), reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2), reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const int maxlen = 80;
    char t1[maxlen + 1];
    for (int len = 17; len < maxlen; len++) {
        memset(t1, 'b', len + 1);
        t1[len - 1] = 'a';

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1),
                                        reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1 + len - 1), rv);
    }
}

TEST(ReverseShufti, ExecNoMatch1) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    char t[] = " bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char *t1 = t + 1;
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t), rv);
    }
}

TEST(ReverseShufti, ExecNoMatch2) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');
    chars.set('B');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    char t[] = " bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char *t1 = t + 1;
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t), rv);
    }
}

TEST(ReverseShufti, ExecNoMatch3) {
    m128 lo, hi;

    CharReach chars;
    chars.set('V'); /* V = 0x56, e = 0x65 */

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    char t[] = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
    char *t1 = t + 1;
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t), rv);
    }
}

TEST(ReverseShufti, ExecMatch1) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbabbbbbbbbbbabbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 17, rv);
    }
}

TEST(ReverseShufti, ExecMatch2) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbabbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 32, rv);
    }
}

TEST(ReverseShufti, ExecMatch3) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');
    chars.set('B');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaBbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
        EXPECT_EQ('B', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 32, rv);
    }

    // check that we match the 'a' bytes as well.
    ASSERT_EQ('B', t1[32]);
    t1[32] = 'b';
    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        ASSERT_NE(reinterpret_cast<const u8 *>(t1) - 1, rv); // not found
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 31, rv);
    }
}

TEST(ReverseShufti, ExecMatch4) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');
    chars.set('C');
    chars.set('A');
    chars.set('c');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    /*          0123456789012345678901234567890 */
    char t1[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaAbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t2[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaCbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t3[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaacbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    char t4[] = "bbbbbbbbbbbbbbbbbaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < 16; i++) {
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len - i);
        EXPECT_EQ('A', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + 32, rv);

        rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t2), reinterpret_cast<u8 *>(t2) + len - i);
        EXPECT_EQ('C', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t2) + 32, rv);

        rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t3), reinterpret_cast<u8 *>(t3) + len - i);
        EXPECT_EQ('c', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t3) + 32, rv);

        rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t4), reinterpret_cast<u8 *>(t4) + len - i);
        EXPECT_EQ('a', (char)*rv);
        ASSERT_EQ(reinterpret_cast<const u8 *>(t4) + 32, rv);
    }
}

TEST(ReverseShufti, ExecMatch5) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    char t1[] = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    size_t len = strlen(t1);

    for (size_t i = 0; i < len; i++) {
        t1[i] = 'a';
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + i, rv);
    }
}

TEST(ReverseShufti, ExecMatch6) {
    m128 lo, hi;

    CharReach chars;
    chars.set('a');

    int ret = shuftiBuildMasks(chars, reinterpret_cast<u8 *>(&lo), reinterpret_cast<u8 *>(&hi));
    ASSERT_NE(-1, ret);

    const size_t len = 256;
    char t1[len];
    memset(t1, 'b', len);

    for (size_t i = 0; i < len; i++) {
        t1[i] = 'a';
        DEBUG_PRINTF("i=%ld\n", i);
        const u8 *rv = rshuftiExec(lo, hi, reinterpret_cast<u8 *>(t1), reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1) + i, rv);
    }
}

// Test that having the first char of a two-byte pair at the last position of a
// short buffer (shorter than a SIMD/SVE vector) does not produce a false match.
// This is a regression test for an SVE bug where inactive vector lanes in
// doubleMatched() were not properly masked by the predicate, causing false
// positives when the last byte matched a first-char pattern and the inactive
// lane (zero-filled) satisfied a null-byte second-char pattern.
TEST(DoubleShufti, ExecNoMatchLastByteShortBufNullPair) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;

    // Use a pattern where the second char is the null byte. This ensures
    // mask2_lo[0] and mask2_hi[0] both have the bucket bit cleared, which
    // causes inactive SVE lanes (loaded as 0) to produce t != 0xff.
    lits.insert(make_pair('a', '\0'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits,
                                      reinterpret_cast<u8 *>(&lo1),
                                      reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2),
                                      reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    // Use a large backing buffer filled with 'b' to ensure safe memory reads
    // past buf_end on platforms that use unaligned vector loads (SIMD).
    const int maxlen = 128;
    char t1[maxlen];
    memset(t1, 'b', maxlen);

    // For short buffers (length 2 to 15), place 'a' at the last position.
    // Since there is no '\0' following it within the buffer, no match should
    // be reported.
    for (int len = 2; len <= 15; len++) {
        memset(t1, 'b', maxlen);
        t1[len - 1] = 'a';

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1),
                                        reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1 + len - 1), rv)
            << "Failed for len=" << len;
    }
}

// Same as above, but also test medium-length buffers (16-80) where the tail
// portion processed with a partial predicate may expose the same issue.
TEST(DoubleShufti, ExecNoMatchLastByteNullPairVaryLen) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;
    lits.insert(make_pair('a', '\0'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits,
                                      reinterpret_cast<u8 *>(&lo1),
                                      reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2),
                                      reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const int maxlen = 256;
    char t1[maxlen];

    for (int len = 2; len < maxlen; len++) {
        memset(t1, 'b', maxlen);
        t1[len - 1] = 'a';

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1),
                                        reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1 + len - 1), rv)
            << "Failed for len=" << len;
    }
}

// Verify that a real match of ('a', '\0') within a short buffer IS found.
TEST(DoubleShufti, ExecMatchNullPairShortBuf) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;
    lits.insert(make_pair('a', '\0'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits,
                                      reinterpret_cast<u8 *>(&lo1),
                                      reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2),
                                      reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const int maxlen = 128;
    char t1[maxlen];

    for (int len = 3; len <= 15; len++) {
        memset(t1, 'b', maxlen);
        // Place the pair ('a', '\0') inside the buffer
        t1[len - 2] = 'a';
        t1[len - 1] = '\0';

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1),
                                        reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1 + len - 2), rv)
            << "Match not found for len=" << len;
    }
}

// Test short buffers with a normal two-byte pattern where first char is at the
// last position. This should not match on any platform.
TEST(DoubleShufti, ExecNoMatchLastByteShortBuf) {
    m128 lo1, hi1, lo2, hi2;

    flat_set<pair<u8, u8>> lits;
    lits.insert(make_pair('x', 'y'));

    bool ret = shuftiBuildDoubleMasks(CharReach(), lits,
                                      reinterpret_cast<u8 *>(&lo1),
                                      reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2),
                                      reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const int maxlen = 128;
    char t1[maxlen];

    for (int len = 2; len <= 15; len++) {
        memset(t1, 'b', maxlen);
        t1[len - 1] = 'x'; // first char at last position, no 'y' follows

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1),
                                        reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1 + len - 1), rv)
            << "Failed for len=" << len;
    }
}

// Test short buffers with mixed one-byte and two-byte patterns. A one-byte
// match at the last position should be correctly reported.
TEST(DoubleShufti, ExecMatchMixedShortBuf) {
    m128 lo1, hi1, lo2, hi2;

    CharReach onebyte;
    flat_set<pair<u8, u8>> twobyte;

    onebyte.set('a');
    twobyte.insert(make_pair('x', 'y'));

    bool ret = shuftiBuildDoubleMasks(onebyte, twobyte,
                                      reinterpret_cast<u8 *>(&lo1),
                                      reinterpret_cast<u8 *>(&hi1),
                                      reinterpret_cast<u8 *>(&lo2),
                                      reinterpret_cast<u8 *>(&hi2));
    ASSERT_TRUE(ret);

    const int maxlen = 128;
    char t1[maxlen];

    // One-byte 'a' at the last position should be reported
    for (int len = 2; len <= 15; len++) {
        memset(t1, 'b', maxlen);
        t1[len - 1] = 'a';

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1),
                                        reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1 + len - 1), rv)
            << "One-byte match not found for len=" << len;
    }

    // No match when target chars are absent
    for (int len = 2; len <= 15; len++) {
        memset(t1, 'b', maxlen);

        const u8 *rv = shuftiDoubleExec(lo1, hi1, lo2, hi2,
                                        reinterpret_cast<u8 *>(t1),
                                        reinterpret_cast<u8 *>(t1) + len);

        ASSERT_EQ(reinterpret_cast<const u8 *>(t1 + len), rv)
            << "False match for len=" << len;
    }
}
