/*
 * Copyright (c) 2018-2019, Intel Corporation
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

#include <algorithm>
#include <array>
#include <iostream>
#include <vector>

#include "gtest/gtest.h"
#include "hs.h"
#include "config.h"
#include "test_util.h"


#include <fstream>
#include <sstream>
#include <string>

using namespace std;

#define xstr(s) to_string_literal(s)
#define to_string_literal(s) #s

#define SRCDIR_PREFIX xstr(SRCDIR)


TEST(rebar, leipzig_math_symbols_count) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    CallBackContext c;
    const char *expr = "\\p{Sm}";
    const unsigned flag = HS_FLAG_UCP | HS_FLAG_UTF8;
    hs_error_t err = hs_compile(expr, flag, HS_MODE_BLOCK,nullptr, &db, &compile_err);

    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(scratch != nullptr);

    string filename = "unit/hyperscan/datafiles/leipzig-3200.txt";
    std::ifstream file((string(SRCDIR_PREFIX) + "/" + filename).c_str());
    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the file into the buffer
    std::string data = buffer.str(); // Convert the buffer into a std::string

    c.halt = 0;
    err = hs_scan(db, data.c_str(), data.size(), 0, scratch, record_cb,
                  reinterpret_cast<void *>(&c));
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_EQ(69, c.matches.size());

    hs_free_database(db);
    err = hs_free_scratch(scratch);
    ASSERT_EQ(HS_SUCCESS, err);
}

// Function to replace invalid UTF-8 sequences with the replacement character
std::string utf8_lossy_decode(const std::string &input) {
    std::string output;
    for (size_t i = 0; i < input.size(); ++i) {
        unsigned char c = input[i];
        if (c < 0x80) {
            output += c;
        } else if (c < 0xC0) {
            output += '\xEF';
            output += '\xBF';
            output += '\xBD';
        } else if (c < 0xE0) {
            if (i + 1 < input.size() && (input[i + 1] & 0xC0) == 0x80) {
                output += c;
                output += input[i + 1];
                ++i;
            } else {
                output += '\xEF';
                output += '\xBF';
                output += '\xBD';
            }
        } else if (c < 0xF0) {
            if (i + 2 < input.size() && (input[i + 1] & 0xC0) == 0x80 && (input[i + 2] & 0xC0) == 0x80) {
                output += c;
                output += input[i + 1];
                output += input[i + 2];
                i += 2;
            } else {
                output += '\xEF';
                output += '\xBF';
                output += '\xBD';
            }
        } else {
            output += '\xEF';
            output += '\xBF';
            output += '\xBD';
        }
    }
    return output;
}

TEST(rebar, lh3lh3_reb_uri_or_email_grep) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    CallBackContext c;
    const char *expr = "([a-zA-Z][a-zA-Z0-9]*)://([^ /]+)(/[^ ]*)?|([^ @]+)@([^ @]+)";
    const unsigned flag = 0;
    hs_error_t err = hs_compile(expr, flag, HS_MODE_BLOCK, nullptr, &db, &compile_err);

    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(scratch != nullptr);

    string filename = "unit/hyperscan/datafiles/lh3lh3-reb-howto.txt";
    std::ifstream file((string(SRCDIR_PREFIX) + "/" + filename).c_str());
    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the file into the buffer
    std::string data = buffer.str(); // Convert the buffer into a std::string

    // Decode the data using UTF-8 lossy decoding
    std::string decoded_data = utf8_lossy_decode(data);

    c.halt = 0;
    err = hs_scan(db, decoded_data.c_str(), decoded_data.size(), 0, scratch, record_cb,
                  reinterpret_cast<void *>(&c));
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_EQ(888987, c.matches.size());

    hs_free_database(db);
    err = hs_free_scratch(scratch);
    ASSERT_EQ(HS_SUCCESS, err);
}

TEST(rebar, lh3lh3_reb_email_grep) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    CallBackContext c;
    const char *expr = "([^ @]+)@([^ @]+)";
    const unsigned flag = 0;
    hs_error_t err = hs_compile(expr, flag, HS_MODE_BLOCK, nullptr, &db, &compile_err);

    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(scratch != nullptr);

    string filename = "unit/hyperscan/datafiles/lh3lh3-reb-howto.txt";
    std::ifstream file((string(SRCDIR_PREFIX) + "/" + filename).c_str());
    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the file into the buffer
    std::string data = buffer.str(); // Convert the buffer into a std::string

    // Decode the data using UTF-8 lossy decoding
    std::string decoded_data = utf8_lossy_decode(data);

    c.halt = 0;
    err = hs_scan(db, decoded_data.c_str(), decoded_data.size(), 0, scratch, record_cb,
                  reinterpret_cast<void *>(&c));
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_EQ(232354, c.matches.size());

    hs_free_database(db);
    err = hs_free_scratch(scratch);
    ASSERT_EQ(HS_SUCCESS, err);
}


TEST(rebar, lh3lh3_reb_date_grep) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    CallBackContext c;
    const char *expr = "([0-9][0-9]?)/([0-9][0-9]?)/([0-9][0-9]([0-9][0-9])?)";
    const unsigned flag = 0;
    hs_error_t err = hs_compile(expr, flag, HS_MODE_BLOCK,nullptr, &db, &compile_err);

    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(scratch != nullptr);


    string filename = "unit/hyperscan/datafiles/lh3lh3-reb-howto.txt";
    std::ifstream file((string(SRCDIR_PREFIX) + "/" + filename).c_str());
    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the file into the buffer
    std::string data = buffer.str(); // Convert the buffer into a std::string
    std::string decoded_data = utf8_lossy_decode(data);
    c.halt = 0;
    err = hs_scan(db, decoded_data.c_str(), decoded_data.size(), 0, scratch, record_cb,
                  reinterpret_cast<void *>(&c));
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_EQ(819, c.matches.size());

    hs_free_database(db);
    err = hs_free_scratch(scratch);
    ASSERT_EQ(HS_SUCCESS, err);
}


const char *patterns[] = {
    "^muvoy-nyemcynjywynamlahi/nyzye/khjdrehko-(qjhn|lyol)-.*/0$",
    "^cop/devel/workflows-(prod|test)-.*/[0-9]+$", // Regex pattern that will match our fixture
    
};

TEST(bug317, regressionOnx86Bug317) {
    hs_database_t *database;
    hs_compile_error_t *compile_err;    

    unsigned ids[2] = {0};
    ids[0]=0;
    ids[1]=1;
    
    const unsigned flag = HS_FLAG_SINGLEMATCH | HS_FLAG_ALLOWEMPTY | HS_FLAG_UTF8 | HS_FLAG_PREFILTER;
    std::vector<unsigned> flags;
    for (size_t i = 0; i < 2; ++i) {
        flags.push_back(flag);
    } 
    hs_error_t err = hs_compile_multi(patterns, flags.data(), ids, 2, HS_MODE_BLOCK, NULL, &database, &compile_err);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(database != nullptr);

    // Allocate scratch space
    hs_scratch_t *scratch = NULL;
    err = hs_alloc_scratch(database, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(scratch != nullptr);
    // This input should match
    const char* input = "cop/devel/workflows-prod-build-cop-cop-ingestor/0";
    
    // Scan the input
    bool matchFound = false;
    auto matchHandler = [](unsigned int, unsigned long long, unsigned long long, unsigned int, void *ctx) -> int {
        bool *matchFound = static_cast<bool*>(ctx);
        *matchFound = true;
        return 0;
    };

    err= hs_scan(database, input, strlen(input), 0, scratch, matchHandler, reinterpret_cast<void *>(&matchFound));
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_EQ(true, matchFound);
    // Clean up
    hs_free_database(database);
    err = hs_free_scratch(scratch);
}

TEST(ArmRegression, FalseNegativeWithBackslashS_Short) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    
    const char *pattern = "curl.*\\|\\s*python";
    hs_error_t err = hs_compile(pattern, HS_FLAG_DOTALL, HS_MODE_BLOCK, 
                                 nullptr, &db, &compile_err);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);

    const char *input = "curl http://xxx:8000 2>&1 | python";
    
    bool matched = false;
    auto cb = [](unsigned int, unsigned long long, unsigned long long, 
                 unsigned int, void *ctx) -> int {
        *static_cast<bool*>(ctx) = true;
        return 0;
    };

    err = hs_scan(db, input, strlen(input), 0, scratch, cb, &matched);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(matched) << "Pattern should match input with 3-char hostname";

    hs_free_scratch(scratch);
    hs_free_database(db);
}

TEST(ArmRegression, FalseNegativeWithBackslashS_Long) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    
    const char *pattern = "curl.*\\|\\s*python";
    hs_error_t err = hs_compile(pattern, HS_FLAG_DOTALL, HS_MODE_BLOCK, 
                                 nullptr, &db, &compile_err);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);

    const char *input = "curl http://xxxx:8000 2>&1 | python";
    
    bool matched = false;
    auto cb = [](unsigned int, unsigned long long, unsigned long long, 
                 unsigned int, void *ctx) -> int {
        *static_cast<bool*>(ctx) = true;
        return 0;
    };

    err = hs_scan(db, input, strlen(input), 0, scratch, cb, &matched);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(matched) << "Pattern should match input with 4-char hostname";

    hs_free_scratch(scratch);
    hs_free_database(db);
}

TEST(ArmRegression, FalseNegativeWithBackslashS_RealWorld) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    
    const char *pattern = "curl.*\\|\\s*python";
    hs_error_t err = hs_compile(pattern, HS_FLAG_DOTALL, HS_MODE_BLOCK, 
                                 nullptr, &db, &compile_err);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);

    const char *input = "curl http://vader:8080 2>&1 | python";
    
    bool matched = false;
    auto cb = [](unsigned int, unsigned long long, unsigned long long, 
                 unsigned int, void *ctx) -> int {
        *static_cast<bool*>(ctx) = true;
        return 0;
    };

    err = hs_scan(db, input, strlen(input), 0, scratch, cb, &matched);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(matched) << "Pattern should match real-world curl|python input";

    hs_free_scratch(scratch);
    hs_free_database(db);
}

TEST(ArmRegression, FalseNegativeWithBackslashS_CharClassAlsoFails) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    
    const char *pattern = "curl.*\\|[ \\t]*python";
    hs_error_t err = hs_compile(pattern, HS_FLAG_DOTALL, HS_MODE_BLOCK, 
                                 nullptr, &db, &compile_err);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);

    const char *input = "curl http://xxxx:8000 2>&1 | python";
    
    bool matched = false;
    auto cb = [](unsigned int, unsigned long long, unsigned long long, 
                 unsigned int, void *ctx) -> int {
        *static_cast<bool*>(ctx) = true;
        return 0;
    };

    err = hs_scan(db, input, strlen(input), 0, scratch, cb, &matched);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(matched) << "Pattern with [ \\t]* should match (but fails due to bug)";

    hs_free_scratch(scratch);
    hs_free_database(db);
}

TEST(ArmRegression, NoDotAll_Short) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    
    const char *pattern = "curl.*\\|\\s*python";
    hs_error_t err = hs_compile(pattern, 0, HS_MODE_BLOCK, 
                                 nullptr, &db, &compile_err);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);

    const char *input = "curl http://xxx:8000 2>&1 | python";
    
    bool matched = false;
    auto cb = [](unsigned int, unsigned long long, unsigned long long, 
                 unsigned int, void *ctx) -> int {
        *static_cast<bool*>(ctx) = true;
        return 0;
    };

    err = hs_scan(db, input, strlen(input), 0, scratch, cb, &matched);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(matched) << "Without DOTALL: 3-char hostname should match";

    hs_free_scratch(scratch);
    hs_free_database(db);
}

TEST(ArmRegression, NoDotAll_Long) {
    hs_database_t *db = nullptr;
    hs_compile_error_t *compile_err = nullptr;
    
    const char *pattern = "curl.*\\|\\s*python";
    hs_error_t err = hs_compile(pattern, 0, HS_MODE_BLOCK, 
                                 nullptr, &db, &compile_err);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(db != nullptr);

    hs_scratch_t *scratch = nullptr;
    err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);

    const char *input = "curl http://xxxx:8000 2>&1 | python";
    
    bool matched = false;
    auto cb = [](unsigned int, unsigned long long, unsigned long long, 
                 unsigned int, void *ctx) -> int {
        *static_cast<bool*>(ctx) = true;
        return 0;
    };

    err = hs_scan(db, input, strlen(input), 0, scratch, cb, &matched);
    ASSERT_EQ(HS_SUCCESS, err);
    ASSERT_TRUE(matched) << "Without DOTALL: 4-char hostname should match";

    hs_free_scratch(scratch);
    hs_free_database(db);
}


TEST(utf8, charclass_issue_326) {
    /*
     * This is a modified test case from https://github.com/VectorCamp/vectorscan/issues/326
     * It includes both test patterns mentioned in the issue.
     */
    vector<pattern> unicode_patterns = {
    pattern(R"(\x{ff15}\x{ff10}\x{ff17}\x{ff15}\x{ff10}[\x{ff10}-\x{ff19}]{7})",
        HS_FLAG_DOTALL | HS_FLAG_PREFILTER | HS_FLAG_MULTILINE | HS_FLAG_CASELESS | HS_FLAG_UCP | HS_FLAG_UTF8, 1),
    pattern(R"(NL[0-9\x{ff10}-\x{ff19}]{2}[A-Z\x{ff21}-\x{ff3a}a-z\x{ff41}-\x{ff5a}]{4}[0-9\x{ff10}-\x{ff19}]{10})",
        HS_FLAG_PREFILTER | HS_FLAG_SINGLEMATCH| HS_FLAG_UTF8, 2)
    };
    const char *data1 = "５０７５０７８３２４０１";
    const char *data2 = "NL２０INGB０００１２３４567";

    hs_database_t *db = buildDB(unicode_patterns, HS_MODE_NOSTREAM);
    ASSERT_NE(nullptr, db);

    hs_scratch_t *scratch = nullptr;
    hs_error_t err = hs_alloc_scratch(db, &scratch);
    ASSERT_EQ(HS_SUCCESS, err);

    CallBackContext c1, c2;
    err = hs_scan(db, data1, strlen(data1), 0, scratch, record_cb, reinterpret_cast<void *>(&c1));
    ASSERT_EQ(HS_SUCCESS, err);
    
    err = hs_scan(db, data2, strlen(data2), 0, scratch, record_cb, reinterpret_cast<void *>(&c2));
    ASSERT_EQ(HS_SUCCESS, err);

    ASSERT_EQ(MatchRecord(36, 1), c1.matches[0]);
    ASSERT_EQ(MatchRecord(36, 2), c2.matches[0]);
    err = hs_free_scratch(scratch);
    ASSERT_EQ(HS_SUCCESS, err);
    hs_free_database(db);
}

TEST(bug339, delayed_fragment_assert) {
    hs_database_t *db = buildDB(R"((?:1\d|3[01])(?:0[1-9]|1[01])\d)", 0, 0, HS_MODE_STREAM);
    ASSERT_NE(nullptr, db);

    hs_free_database(db);
}
