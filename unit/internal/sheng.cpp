/*
 * Copyright (c) 2024, Arm ltd
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
#include "nfa/shengcompile.h"
#include "nfa/rdfa.h"
#include "util/bytecode_ptr.h"
#include "util/compile_context.h"
#include "util/report_manager.h"

extern "C" {
    #include "hs_compile.h"
    #include "nfa/nfa_api.h"
    #include "nfa/nfa_api_queue.h"
    #include "nfa/nfa_api_util.h"
    #include "nfa/nfa_internal.h"
    #include "nfa/rdfa.h"
    #include "nfa/sheng.h"
    #include "ue2common.h"
}

namespace {

struct callback_context {
    unsigned int period;
    unsigned int match_count;
    unsigned int pattern_length;
};

int dummy_callback(u64a start, u64a end, ReportID id, void *context) {
    (void) context;
    printf("callback %llu %llu %u\n", start, end, id);
    return 1; /* 0 stops matching, !0 continue */
}

int periodic_pattern_callback(u64a start, u64a end, ReportID id, void *raw_context) {
    struct callback_context *context = (struct callback_context*) raw_context;
    (void) start;
    (void) id;
    EXPECT_EQ(context->period * context->match_count, end - context->pattern_length);
    context->match_count++;
    return 1; /* 0 stops matching, !0 continue */
}

/**
 * @brief Fill the state matrix with a diagonal pattern: accept the Nth character to go to the N+1 state
 */
static void fill_straight_regex_sequence(struct ue2::raw_dfa *dfa, int start_state, int end_state, int state_count)
{
    for (int state = start_state; state < end_state; state++) {
        dfa->states[state].next.assign(state_count ,1);
        dfa->states[state].next[0] = 2;
        dfa->states[state].next[1] = 2;
        dfa->states[state].next[state] = state+1;
    }
}

static void init_raw_dfa16(struct ue2::raw_dfa *dfa, const ReportID rID)
{
    dfa->start_anchored = 1;
    dfa->start_floating = 1;
    dfa->alpha_size = 8;

    int nb_state = 8;
    for(int i = 0; i < nb_state; i++) {
        struct ue2::dstate state(dfa->alpha_size);
        state.next = std::vector<ue2::dstate_id_t>(nb_state);
        state.daddy = 0;
        state.impl_id = i; /* id of the state */
        state.reports = ue2::flat_set<ReportID>();
        state.reports_eod = ue2::flat_set<ReportID>();
        dfa->states.push_back(state);
    }

    /* add a report to every accept state */
    dfa->states[7].reports.insert(rID);

    /**
     * [a,b][c-e]{3}of
     * (1) -a,b-> (2) -c,d,e-> (3) -c,d,e-> (4) -c,d,e-> (5) -o-> (6) -f-> ((7))
     * (0) = dead
     */

    for(int i = 0; i < ue2::ALPHABET_SIZE; i++) {
        dfa->alpha_remap[i] = 0;
    }

    dfa->alpha_remap['a'] = 0;
    dfa->alpha_remap['b'] = 1;
    dfa->alpha_remap['c'] = 2;
    dfa->alpha_remap['d'] = 3;
    dfa->alpha_remap['e'] = 4;
    dfa->alpha_remap['o'] = 5;
    dfa->alpha_remap['f'] = 6;
    dfa->alpha_remap[256] = 7; /* for some reason there's a check that run on dfa->alpha_size-1 */

                        /* a b c d e o f */
    dfa->states[0].next = {0,0,0,0,0,0,0};
    dfa->states[1].next = {2,2,1,1,1,1,1};      /* nothing */
    dfa->states[2].next = {2,2,3,3,3,1,1};      /* [a,b] */
    dfa->states[3].next = {2,2,4,4,4,1,1};      /* [a,b][c-e]{1} */
    dfa->states[4].next = {2,2,5,5,5,1,1};      /* [a,b][c-e]{2} */
    fill_straight_regex_sequence(dfa, 5, 7, 7); /* [a,b][c-e]{3}o */
    dfa->states[7].next = {2,2,1,1,1,1,1};      /* [a,b][c-e]{3}of */
}

#if defined(HAVE_AVX512VBMI) || defined(HAVE_SVE)
/* We need more than 16 states to run sheng32, so make the graph longer */
static void init_raw_dfa32(struct ue2::raw_dfa *dfa, const ReportID rID)
{
    dfa->start_anchored = 1;
    dfa->start_floating = 1;
    dfa->alpha_size = 18;

    int nb_state = 18;
    for(int i = 0; i < nb_state; i++) {
        struct ue2::dstate state(dfa->alpha_size);
        state.next = std::vector<ue2::dstate_id_t>(nb_state);
        state.daddy = 0;
        state.impl_id = i; /* id of the state */
        state.reports = ue2::flat_set<ReportID>();
        state.reports_eod = ue2::flat_set<ReportID>();
        dfa->states.push_back(state);
    }

    /* add a report to every accept state */
    dfa->states[17].reports.insert(rID);

    /**
     * [a,b][c-e]{3}of0123456789
     * (1) -a,b-> (2) -c,d,e-> (3) -c,d,e-> (4) -c,d,e-> (5) -o-> (6) -f-> (7) -<numbers>-> ((17))
     * (0) = dead
     */

    for(int i = 0; i < ue2::ALPHABET_SIZE; i++) {
        dfa->alpha_remap[i] = 0;
    }

    dfa->alpha_remap['a'] = 0;
    dfa->alpha_remap['b'] = 1;
    dfa->alpha_remap['c'] = 2;
    dfa->alpha_remap['d'] = 3;
    dfa->alpha_remap['e'] = 4;
    dfa->alpha_remap['o'] = 5;
    dfa->alpha_remap['f'] = 6;
    // maps 0 to 9
    for (int i = 0; i < 10; i ++) {
        dfa->alpha_remap[i + '0'] = i + 7;
    }
    dfa->alpha_remap[256] = 17; /* for some reason there's a check that run on dfa->alpha_size-1 */

                         /* a b c d e o f 0 1 2 3 4 5 6 7 8 9 */
    dfa->states[0].next  = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    dfa->states[1].next  = {2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};  /* nothing */
    dfa->states[2].next  = {2,2,3,3,3,1,1,1,1,1,1,1,1,1,1,1,1};  /* [a,b] */
    dfa->states[3].next  = {2,2,4,4,4,1,1,1,1,1,1,1,1,1,1,1,1};  /* [a,b][c-e]{1} */
    dfa->states[4].next  = {2,2,5,5,5,1,1,1,1,1,1,1,1,1,1,1,1};  /* [a,b][c-e]{2} */
    fill_straight_regex_sequence(dfa, 5, 17, 17);                /* [a,b][c-e]{3}of012345678 */
    dfa->states[17].next = {2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};  /* [a,b][c-e]{3}of0123456789 */
}
#endif /* defined(HAVE_AVX512VBMI) || defined(HAVE_SVE) */

typedef ue2::bytecode_ptr<NFA> (*sheng_compile_ptr)(ue2::raw_dfa&,
                            const ue2::CompileContext&,
                            const ue2::ReportManager&,
                            bool,
                            std::set<ue2::dstate_id_t>*);

typedef void (*init_raw_dfa_ptr)(struct ue2::raw_dfa*, const ReportID);


static inline void init_nfa(struct NFA **out_nfa, sheng_compile_ptr compile_function, init_raw_dfa_ptr init_dfa_function) {
    ue2::Grey *g = new ue2::Grey();
#if defined(HAVE_AVX512VBMI)
    hs_platform_info plat_info = {0, HS_CPU_FEATURES_AVX512VBMI, 0, 0};
#else
    hs_platform_info plat_info = {0, 0, 0, 0};
#endif
    ue2::CompileContext *cc = new ue2::CompileContext(false, false, ue2::target_t(plat_info), *g);
    ue2::ReportManager *rm = new ue2::ReportManager(*g);
    ue2::Report *report = new ue2::Report(ue2::EXTERNAL_CALLBACK, 0);
    ReportID rID = rm->getInternalId(*report);
    rm->setProgramOffset(0, 0);

    struct ue2::raw_dfa *dfa = new ue2::raw_dfa(ue2::NFA_OUTFIX);
    init_dfa_function(dfa, rID);

    *out_nfa = (compile_function(*dfa, *cc, *rm, false, nullptr)).release();
    ASSERT_NE(nullptr, *out_nfa);

    delete report;
    delete rm;
    delete cc;
    delete g;
}

static void init_nfa16(struct NFA **out_nfa) {
    init_nfa(out_nfa, ue2::shengCompile, init_raw_dfa16);
}

#if defined(HAVE_AVX512VBMI) || defined(HAVE_SVE)
static void init_nfa32(struct NFA **out_nfa) {
    init_nfa(out_nfa, ue2::sheng32Compile, init_raw_dfa32);
}
#endif /* defined(HAVE_AVX512VBMI) || defined(HAVE_SVE) */

static char state_buffer;

static inline void init_sheng_queue(struct mq **out_q, uint8_t *buffer, size_t max_size, void (*init_nfa_func)(struct NFA **out_nfa) ) {
    struct NFA* nfa;
    init_nfa_func(&nfa);
    assert(nfa);

    struct mq *q = new mq();

    memset(q, 0, sizeof(struct mq));
    q->nfa = nfa;
    q->state = &state_buffer;
    q->cb = dummy_callback;
    q->buffer = buffer;
    q->length = max_size; /* setting this as the max length scanable */

    if (nfa != q->nfa) {
        printf("Something went wrong while initializing sheng.\n");
    }
    nfaQueueInitState(nfa, q);
    pushQueueAt(q, 0, MQE_START, 0);
    pushQueueAt(q, 1, MQE_END, q->length );

    *out_q = q;
}

static void init_sheng_queue16(struct mq **out_q, uint8_t *buffer ,size_t max_size) {
    init_sheng_queue(out_q, buffer, max_size, init_nfa16);
}

#if defined(HAVE_AVX512VBMI) || defined(HAVE_SVE)
static void init_sheng_queue32(struct mq **out_q, uint8_t *buffer, size_t max_size) {
    init_sheng_queue(out_q, buffer, max_size, init_nfa32);
}
#endif /* defined(HAVE_AVX512VBMI) || defined(HAVE_SVE) */

static
void fill_pattern(u8* buf, size_t buffer_size, unsigned int start_offset, unsigned int period, const char *pattern, unsigned int pattern_length) {
    memset(buf, '_', buffer_size);

    for (unsigned int i = 0; i < buffer_size - 8; i+= 8) {
        /* filling with some junk, including some character used for a valid state, to prevent the use of shufti */
        memcpy(buf + i, "jgohcxbf", 8); 
    }

    for (unsigned int i = start_offset; i < buffer_size - pattern_length; i += period) {
        memcpy(buf + i, pattern, pattern_length);
    }
}

/* Generate ground truth to compare to */
struct NFA *get_expected_nfa_header(u8 type, unsigned int length, unsigned int nposition) {
    struct NFA *expected_nfa_header = new struct NFA();
    memset(expected_nfa_header, 0, sizeof(struct NFA));
    expected_nfa_header->length = length;
    expected_nfa_header->type = type;
    expected_nfa_header->nPositions = nposition;
    expected_nfa_header->scratchStateSize = 1;
    expected_nfa_header->streamStateSize = 1;
    return expected_nfa_header;
}

struct NFA *get_expected_nfa16_header() {
    return get_expected_nfa_header(SHENG_NFA, 4736, 8); /* size recorded in 04/2024 */
}

#if defined(HAVE_AVX512VBMI) || defined(HAVE_SVE)
struct NFA *get_expected_nfa32_header() {
    return get_expected_nfa_header(SHENG_NFA_32, 17216, 18); /* size recorded in 04/2024 */
}
#endif /* defined(HAVE_AVX512VBMI) || defined(HAVE_SVE) */

void test_nfa_equal(const NFA& l, const NFA& r)
{
    /**
     * The length is meant to be a sanity test: it's not 0 (we compiled something) and that it roughly fit the
     * expected size for a given sheng implementation (we don't feed compiled sheng32 into sheng16).
     * Changes in other nfa algorithms may affect the sheng length, so we accept small variations.
     */
    int relative_difference = std::abs((float)(l.length) - r.length) / ((l.length + r.length) / 2);
    EXPECT_LE(relative_difference, 0.1); /* same +-10% */

    EXPECT_EQ(l.flags, r.flags);
    EXPECT_EQ(l.type, r.type);
    EXPECT_EQ(l.rAccelType, r.rAccelType);
    EXPECT_EQ(l.rAccelOffset, r.rAccelOffset);
    EXPECT_EQ(l.maxBiAnchoredWidth, r.maxBiAnchoredWidth);
    EXPECT_EQ(l.rAccelData.dc, r.rAccelData.dc);
    EXPECT_EQ(l.queueIndex, r.queueIndex);
    EXPECT_EQ(l.nPositions, r.nPositions);
    EXPECT_EQ(l.scratchStateSize, r.scratchStateSize);
    EXPECT_EQ(l.streamStateSize, r.streamStateSize);
    EXPECT_EQ(l.maxWidth, r.maxWidth);
    EXPECT_EQ(l.minWidth, r.minWidth);
    EXPECT_EQ(l.maxOffset, r.maxOffset);
}

/* Start of actual tests */

/* 
 * Runs shengCompile and compares its outputs to previously recorded outputs.
 */
TEST(Sheng16, std_compile_header) {

    ue2::Grey *g = new ue2::Grey();
    hs_platform_info plat_info = {0, 0, 0, 0};
    ue2::CompileContext *cc = new ue2::CompileContext(false, false, ue2::target_t(plat_info), *g);
    ue2::ReportManager *rm = new ue2::ReportManager(*g);
    ue2::Report *report = new ue2::Report(ue2::EXTERNAL_CALLBACK, 0);
    ReportID rID = rm->getInternalId(*report);
    rm->setProgramOffset(0, 0);

    struct ue2::raw_dfa *dfa = new ue2::raw_dfa(ue2::NFA_OUTFIX);
    init_raw_dfa16(dfa, rID);

    struct NFA *nfa = (shengCompile(*dfa, *cc, *rm, false)).release();
    EXPECT_NE(nullptr, nfa);

    EXPECT_NE(0, nfa->length);
    EXPECT_EQ(SHENG_NFA, nfa->type);

    struct NFA *expected_nfa = get_expected_nfa16_header();
    test_nfa_equal(*expected_nfa, *nfa);

    delete expected_nfa;
    delete report;
    delete rm;
    delete cc;
    delete g;
}

/*
 * nfaExecSheng_B is the most basic of the sheng variants. It simply calls the core of the algorithm.
 * We test it with a buffer having a few matches at fixed intervals and check that it finds them all.
 */
TEST(Sheng16, std_run_B) {
    struct mq *q;
    unsigned int pattern_length = 6;
    unsigned int period = 128;
    const size_t buf_size = 200;
    unsigned int expected_matches = buf_size/128 + 1;
    u8 buf[buf_size];
    struct callback_context context = {period, 0, pattern_length};

    struct NFA* nfa;
    init_nfa16(&nfa);
    ASSERT_NE(nullptr, nfa);
    fill_pattern(buf, buf_size, 0, period, "acecof", pattern_length);
    char ret_val;
    unsigned int offset = 0;
    unsigned int loop_count = 0;
    for (; loop_count < expected_matches + 1; loop_count++) {
        ASSERT_LT(offset, buf_size);
        ret_val = nfaExecSheng_B(nfa,
                                offset,
                                buf + offset,
                                (s64a) buf_size - offset,
                                periodic_pattern_callback,
                                &context);
        offset = (context.match_count - 1) * context.period + context.pattern_length;
        if(unlikely(ret_val != MO_ALIVE)) {
            break;
        }
    }

    /*check normal return*/
    EXPECT_EQ(MO_ALIVE, ret_val);

    /*check that we don't find additional match nor crash when no match are found*/
    EXPECT_EQ(expected_matches + 1, loop_count);

    /*check that we have all the matches*/
    EXPECT_EQ(expected_matches, context.match_count);
}

/*
 * nfaExecSheng_Q runs like the _B version (callback), but exercises the message queue logic.
 * We test it with a buffer having a few matches at fixed intervals and check that it finds them all.
 */
TEST(Sheng16, std_run_Q) {
    struct mq *q;
    unsigned int pattern_length = 6;
    unsigned int period = 128;
    const size_t buf_size = 200;
    unsigned int expected_matches = buf_size/128 + 1;
    u8 buf[buf_size];
    struct callback_context context = {period, 0, pattern_length};

    init_sheng_queue16(&q, buf, buf_size);
    fill_pattern(buf, buf_size, 0, period, "acecof", pattern_length);
    q->cur = 0;
    q->items[q->cur].location = 0;
    q->context = &context;
    q->cb = periodic_pattern_callback;

    nfaExecSheng_Q(q->nfa, q, (s64a) buf_size);
    /*check that we have all the matches*/
    EXPECT_EQ(expected_matches, context.match_count);

    delete q;
}

/*
 * nfaExecSheng_Q2 uses the message queue, but stops at match instead of using a callback.
 * We test it with a buffer having a few matches at fixed intervals and check that it finds them all.
 */
TEST(Sheng16, std_run_Q2) {
    struct mq *q;
    unsigned int pattern_length = 6;
    unsigned int period = 128;
    const size_t buf_size = 200;
    unsigned int expected_matches = buf_size/128 + 1;
    u8 buf[buf_size];

    init_sheng_queue16(&q, buf, buf_size);
    fill_pattern(buf, buf_size, 0, period, "acecof", pattern_length);
    q->cur = 0;
    q->items[q->cur].location = 0;

    char ret_val;
    int location;
    unsigned int loop_count = 0;
    do {
        ret_val = nfaExecSheng_Q2(q->nfa, q, (s64a) buf_size);
        location = q->items[q->cur].location;
        loop_count++;
    } while(likely((ret_val == MO_MATCHES_PENDING) && (location < (int)buf_size) && ((location % period) == pattern_length)));

    /*check if it's a spurious match*/
    EXPECT_EQ(0, (ret_val == MO_MATCHES_PENDING) && ((location % period) != pattern_length));

    /*check that we have all the matches*/
    EXPECT_EQ(expected_matches, loop_count-1);

    delete q;
}

/*
 * The message queue can also run on the "history" buffer. We test it the same way as the normal 
 * buffer, expecting the same behavior.
 * We test it with a buffer having a few matches at fixed intervals and check that it finds them all.
 */
TEST(Sheng16, history_run_Q2) {
    struct mq *q;
    unsigned int pattern_length = 6;
    unsigned int period = 128;
    const size_t buf_size = 200;
    unsigned int expected_matches = buf_size/128 + 1;
    u8 buf[buf_size];

    init_sheng_queue16(&q, buf, buf_size);
    fill_pattern(buf, buf_size, 0, period, "acecof", pattern_length);
    q->history = buf;
    q->hlength = buf_size;
    q->cur = 0;
    q->items[q->cur].location = -200;

    char ret_val;
    int location;
    unsigned int loop_count = 0;
    do {
        ret_val = nfaExecSheng_Q2(q->nfa, q, 0);
        location = q->items[q->cur].location;
        loop_count++;
    } while(likely((ret_val == MO_MATCHES_PENDING) && (location > -(int)buf_size) && (location < 0) && (((buf_size + location) % period) == pattern_length)));

    /*check if it's a spurious match*/
    EXPECT_EQ(0, (ret_val == MO_MATCHES_PENDING) && (((buf_size + location) % period) != pattern_length));

    /*check that we have all the matches*/
    EXPECT_EQ(expected_matches, loop_count-1);

    delete q;
}

/**
 * Those tests only covers the basic paths. More tests can cover:
 * - running for history buffer to current buffer in Q2
 * - running while expecting no match
 * - nfaExecSheng_QR
 * - run sheng when it should call an accelerator and confirm it call them
 */

#if defined(HAVE_AVX512VBMI) || defined(HAVE_SVE)

/* 
 * Runs sheng32Compile and compares its outputs to previously recorded outputs.
 */
TEST(Sheng32, std_compile_header) {
#if defined(HAVE_SVE)
    if(svcntb()<32) {
        return;
    }
#endif
    ue2::Grey *g = new ue2::Grey();
    hs_platform_info plat_info = {0, HS_CPU_FEATURES_AVX512VBMI, 0, 0};
    ue2::CompileContext *cc = new ue2::CompileContext(false, false, ue2::target_t(plat_info), *g);
    ue2::ReportManager *rm = new ue2::ReportManager(*g);
    ue2::Report *report = new ue2::Report(ue2::EXTERNAL_CALLBACK, 0);
    ReportID rID = rm->getInternalId(*report);
    rm->setProgramOffset(0, 0);

    struct ue2::raw_dfa *dfa = new ue2::raw_dfa(ue2::NFA_OUTFIX);
    init_raw_dfa32(dfa, rID);

    struct NFA *nfa = (sheng32Compile(*dfa, *cc, *rm, false)).release();
    EXPECT_NE(nullptr, nfa);

    EXPECT_NE(0, nfa->length);
    EXPECT_EQ(SHENG_NFA_32, nfa->type);

    struct NFA *expected_nfa = get_expected_nfa32_header();
    test_nfa_equal(*expected_nfa, *nfa);

    delete expected_nfa;
    delete report;
    delete rm;
    delete cc;
    delete g;
}

/*
 * nfaExecSheng32_B is the most basic of the sheng variants. It simply calls the core of the algorithm.
 * We test it with a buffer having a few matches at fixed intervals and check that it finds them all.
 */
TEST(Sheng32, std_run_B) {
#if defined(HAVE_SVE)
    if(svcntb()<32) {
        return;
    }
#endif
    struct mq *q;
    unsigned int pattern_length = 16;
    unsigned int period = 128;
    const size_t buf_size = 200;
    unsigned int expected_matches = buf_size/128 + 1;
    u8 buf[buf_size];
    struct callback_context context = {period, 0, pattern_length};

    struct NFA* nfa;
    init_nfa32(&nfa);
    ASSERT_NE(nullptr, nfa);
    fill_pattern(buf, buf_size, 0, period, "acecof0123456789", pattern_length);
    char ret_val;
    unsigned int offset = 0;
    unsigned int loop_count = 0;
    for (; loop_count < expected_matches + 1; loop_count++) {
        ASSERT_LT(offset, buf_size);
        ret_val = nfaExecSheng32_B(nfa,
                                offset,
                                buf + offset,
                                (s64a) buf_size - offset,
                                periodic_pattern_callback,
                                &context);
        offset = (context.match_count - 1) * context.period + context.pattern_length;
        if(unlikely(ret_val != MO_ALIVE)) {
            break;
        }
    }

    /*check normal return*/
    EXPECT_EQ(MO_ALIVE, ret_val);

    /*check that we don't find additional match nor crash when no match are found*/
    EXPECT_EQ(expected_matches + 1, loop_count);

    /*check that we have all the matches*/
    EXPECT_EQ(expected_matches, context.match_count);
}

/*
 * nfaExecSheng32_Q runs like the _B version (callback), but exercises the message queue logic.
 * We test it with a buffer having a few matches at fixed intervals and check that it finds them all.
 */
TEST(Sheng32, std_run_Q) {
#if defined(HAVE_SVE)
    if(svcntb()<32) {
        return;
    }
#endif
    struct mq *q;
    unsigned int pattern_length = 16;
    unsigned int period = 128;
    const size_t buf_size = 200;
    unsigned int expected_matches = buf_size/128 + 1;
    u8 buf[buf_size];
    struct callback_context context = {period, 0, pattern_length};

    init_sheng_queue32(&q, buf, buf_size);
    fill_pattern(buf, buf_size, 0, period, "acecof0123456789", pattern_length);
    q->cur = 0;
    q->items[q->cur].location = 0;
    q->context = &context;
    q->cb = periodic_pattern_callback;

    nfaExecSheng32_Q(q->nfa, q, (s64a) buf_size);
    /*check that we have all the matches*/
    EXPECT_EQ(expected_matches, context.match_count);

    delete q;
}

/*
 * nfaExecSheng32_Q2 uses the message queue, but stops at match instead of using a callback.
 * We test it with a buffer having a few matches at fixed intervals and check that it finds them all.
 */
TEST(Sheng32, std_run_Q2) {
#if defined(HAVE_SVE)
    if(svcntb()<32) {
        return;
    }
#endif
    struct mq *q;
    unsigned int pattern_length = 16;
    unsigned int period = 128;
    const size_t buf_size = 200;
    unsigned int expected_matches = buf_size/128 + 1;
    u8 buf[buf_size];

    init_sheng_queue32(&q, buf, buf_size);
    fill_pattern(buf, buf_size, 0, period, "acecof0123456789", pattern_length);
    q->cur = 0;
    q->items[q->cur].location = 0;

    char ret_val;
    int location;
    unsigned int loop_count = 0;
    do {
        ret_val = nfaExecSheng32_Q2(q->nfa, q, (s64a) buf_size);
        location = q->items[q->cur].location;
        loop_count++;
    } while(likely((ret_val == MO_MATCHES_PENDING) && (location < (int)buf_size) && ((location % period) == pattern_length)));

    /*check if it's a spurious match*/
    EXPECT_EQ(0, (ret_val == MO_MATCHES_PENDING) && ((location % period) != pattern_length));

    /*check that we have all the matches*/
    EXPECT_EQ(expected_matches, loop_count-1);

    delete q;
}

/*
 * The message queue can also runs on the "history" buffer. We test it the same way as the normal 
 * buffer, expecting the same behavior.
 * We test it with a buffer having a few matches at fixed intervals and check that it finds them all.
 */
TEST(Sheng32, history_run_Q2) {
#if defined(HAVE_SVE)
    if(svcntb()<32) {
        return;
    }
#endif
    struct mq *q;
    unsigned int pattern_length = 16;
    unsigned int period = 128;
    const size_t buf_size = 200;
    unsigned int expected_matches = buf_size/128 + 1;
    u8 buf[buf_size];

    init_sheng_queue32(&q, buf, buf_size);
    fill_pattern(buf, buf_size, 0, period, "acecof0123456789", pattern_length);
    q->history = buf;
    q->hlength = buf_size;
    q->cur = 0;
    q->items[q->cur].location = -200;

    char ret_val;
    int location;
    unsigned int loop_count = 0;
    do {
        ret_val = nfaExecSheng32_Q2(q->nfa, q, 0);
        location = q->items[q->cur].location;
        loop_count++;
    } while(likely((ret_val == MO_MATCHES_PENDING) && (location > -(int)buf_size) && (location < 0) && (((buf_size + location) % period) == pattern_length)));

    /*check if it's a spurious match*/
    EXPECT_EQ(0, (ret_val == MO_MATCHES_PENDING) && (((buf_size + location) % period) != pattern_length));

    /*check that we have all the matches*/
    EXPECT_EQ(expected_matches, loop_count-1);

    delete q;
}
#endif /* defined(HAVE_AVX512VBMI) || defined(HAVE_SVE) */

} /* namespace */
