/*
 * Copyright (c) 2016-2020, Intel Corporation
 * Copyright (c) 2024, VectorCamp PC
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
#include "hs_runtime.h"
#include "ue2common.h"

/* Streamlining the dispatch to eliminate runtime checking/branching:
 * What we want to do is, first call to the function will run the resolve
 * code and set the static resolved/dispatch pointer to point to the
 * correct function. Subsequent calls to the function will go directly to
 * the resolved ptr. The simplest way to accomplish this is, to
 * initially set the pointer to the resolve function.
 * To accomplish this in a manner invisible to the user,
 * we do involve some rather ugly/confusing macros in here.
 * There are four macros that assemble the code for each function
 * we want to dispatch in this manner:
 * CREATE_DISPATCH
 * this generates the declarations for the candidate target functions,
 * for the fat_dispatch function pointer, for the resolve_ function,
 * points the function pointer to the resolve function, and contains
 * most of the definition of the resolve function. The very end of the
 * resolve function is completed by the next macro, because in the
 * CREATE_DISPATCH macro we have the argument list with the arg declarations,
 * which is needed to generate correct function signatures, but we
 * can't generate from this, in a macro, a _call_ to one of those functions.
 * CONNECT_ARGS_1
 * this macro fills in the actual call at the end of the resolve function,
 * with the correct arg list. hence the name connect args.
 * CONNECT_DISPATCH_2
 * this macro likewise gives up the beginning of the definition of the
 * actual entry point function (the 'real name' that's called by the user)
 * but again in the pass-through call, cannot invoke the target without
 * getting the arg list , which is supplied by the final macro,
 * CONNECT_ARGS_3
 *
 */


#if defined(ARCH_IA32) || defined(ARCH_X86_64)
#include "util/arch/x86/cpuid_inline.h"
#include "util/join.h"

#if defined(DISABLE_AVX512_DISPATCH)
#define avx512_ disabled_
#define check_avx512() (0)
#endif

#if defined(DISABLE_AVX512VBMI_DISPATCH)
#define avx512vbmi_ disabled_
#define check_avx512vbmi() (0)
#endif

#if !defined(VS_SIMDE_BACKEND)
#define simde_ error_
#endif

#define CREATE_DISPATCH(RTYPE, NAME, ...)                                      \
    /* error func */                                                           \
    static inline RTYPE JOIN(error_, NAME)(__VA_ARGS__) {                      \
        return (RTYPE)HS_ARCH_ERROR;                                           \
    }                                                                          \
                                                                               \
    /* create defns */                                                         \
    RTYPE JOIN(avx512vbmi_, NAME)(__VA_ARGS__);                                \
    RTYPE JOIN(avx512_, NAME)(__VA_ARGS__);                                    \
    RTYPE JOIN(avx2_, NAME)(__VA_ARGS__);                                      \
    RTYPE JOIN(corei7_, NAME)(__VA_ARGS__);                                    \
    RTYPE JOIN(simde_, NAME)(__VA_ARGS__);                                     \
                                                                               \
    /* dispatch routing pointer for this function */                           \
    /* initially point it at the resolve function */                           \
    static RTYPE JOIN(resolve_, NAME)(__VA_ARGS__);                            \
    static RTYPE (* JOIN(fat_dispatch_, NAME))(__VA_ARGS__) =                  \
        &JOIN(resolve_, NAME);                                                 \
                                                                               \
    /* resolver */                                                             \
    static RTYPE JOIN(resolve_, NAME)(__VA_ARGS__) {                           \
        if (check_avx512vbmi()) {                                              \
            fat_dispatch_ ## NAME = &JOIN(avx512vbmi_, NAME);                  \
        }                                                                      \
        else if (check_avx512()) {                                             \
            fat_dispatch_ ## NAME = &JOIN(avx512_, NAME);                      \
        }                                                                      \
        else if (check_avx2()) {                                               \
            fat_dispatch_ ## NAME = &JOIN(avx2_, NAME);                        \
        }                                                                      \
        else if (check_sse42() && check_popcnt()) {                            \
            fat_dispatch_ ## NAME = &JOIN(corei7_, NAME);                      \
        } else {                                                               \
            /* If no SSE4.2 is available we fallback to SIMDe */               \
            fat_dispatch_ ## NAME = &JOIN(simde_, NAME);                       \
        }                                                                      \



/* the rest of the function is completed in the CONNECT_ARGS_1 macro. */



#elif defined(ARCH_AARCH64)
#include "util/arch/arm/cpuid_inline.h"
#include "util/join.h"

#define CREATE_DISPATCH(RTYPE, NAME, ...)                                      \
    /* create defns */                                                         \
    RTYPE JOIN(sve2_, NAME)(__VA_ARGS__);                                      \
    RTYPE JOIN(sve_, NAME)(__VA_ARGS__);                                       \
    RTYPE JOIN(neon_, NAME)(__VA_ARGS__);                                      \
                                                                               \
    /* error func */                                                           \
    static inline RTYPE JOIN(error_, NAME)(__VA_ARGS__) {                      \
        return (RTYPE)HS_ARCH_ERROR;                                           \
    }                                                                          \
                                                                               \
    /* dispatch routing pointer for this function */                           \
    /* initially point it at the resolve function */                           \
    static RTYPE JOIN(resolve_, NAME)(__VA_ARGS__);                            \
    static RTYPE (* JOIN(fat_dispatch_, NAME))(__VA_ARGS__) =                  \
        &JOIN(resolve_, NAME);                                                 \
                                                                               \
    /* resolver */                                                             \
    static RTYPE JOIN(resolve_, NAME)(__VA_ARGS__) {                           \
        if (check_sve2()) {                                                    \
            fat_dispatch_ ## NAME = &JOIN(sve2_, NAME);                        \
        }                                                                      \
        else if (check_sve()) {                                                \
            fat_dispatch_ ## NAME = &JOIN(sve_, NAME);                         \
        }                                                                      \
        else if (check_neon()) {                                               \
            fat_dispatch_ ## NAME = &JOIN(neon_, NAME);                        \
        } else {                                                               \
            /* anything else is fail */                                        \
            fat_dispatch_ ## NAME = &JOIN(error_, NAME);                       \
        }                                                                      \


/* the rest of the function is completed in the CONNECT_ARGS_1 macro. */


#endif


#define CONNECT_ARGS_1(RTYPE, NAME, ...)                                       \
        return (*fat_dispatch_ ## NAME)(__VA_ARGS__);                          \
    }                                                                          \


#define CONNECT_DISPATCH_2(RTYPE, NAME, ...)                                   \
    /* new function */                                                         \
    HS_PUBLIC_API                                                              \
    RTYPE NAME(__VA_ARGS__) {                                                  \


#define CONNECT_ARGS_3(RTYPE, NAME, ...)                                       \
        return (*fat_dispatch_ ## NAME)(__VA_ARGS__);                          \
    }                                                                          \


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

/* this gets a bit ugly to compose the static redirect functions,
 * as we necessarily need first the typed arg list and then just the arg
 * names, twice in a row, to define the redirect function and the
 * dispatch function call */

CREATE_DISPATCH(hs_error_t, hs_scan, const hs_database_t *db, const char *data,
                unsigned length, unsigned flags, hs_scratch_t *scratch,
                match_event_handler onEvent, void *userCtx);
CONNECT_ARGS_1(hs_error_t, hs_scan, db, data, length, flags, scratch, onEvent, userCtx);
CONNECT_DISPATCH_2(hs_error_t, hs_scan, const hs_database_t *db, const char *data,
                unsigned length, unsigned flags, hs_scratch_t *scratch,
                match_event_handler onEvent, void *userCtx);
CONNECT_ARGS_3(hs_error_t, hs_scan, db, data, length, flags, scratch, onEvent, userCtx);

CREATE_DISPATCH(hs_error_t, hs_stream_size, const hs_database_t *database,
                size_t *stream_size);
CONNECT_ARGS_1(hs_error_t, hs_stream_size, database, stream_size);
CONNECT_DISPATCH_2(hs_error_t, hs_stream_size, const hs_database_t *database,
                size_t *stream_size);
CONNECT_ARGS_3(hs_error_t, hs_stream_size, database, stream_size);

CREATE_DISPATCH(hs_error_t, hs_database_size, const hs_database_t *db,
                size_t *size);
CONNECT_ARGS_1(hs_error_t, hs_database_size, db, size);
CONNECT_DISPATCH_2(hs_error_t, hs_database_size, const hs_database_t *db,
                size_t *size);
CONNECT_ARGS_3(hs_error_t, hs_database_size, db, size);

CREATE_DISPATCH(hs_error_t, dbIsValid, const hs_database_t *db);
CONNECT_ARGS_1(hs_error_t, dbIsValid, db);
CONNECT_DISPATCH_2(hs_error_t, dbIsValid, const hs_database_t *db);
CONNECT_ARGS_3(hs_error_t, dbIsValid, db);

CREATE_DISPATCH(hs_error_t, hs_free_database, hs_database_t *db);
CONNECT_ARGS_1(hs_error_t, hs_free_database, db);
CONNECT_DISPATCH_2(hs_error_t, hs_free_database, hs_database_t *db);
CONNECT_ARGS_3(hs_error_t, hs_free_database, db);

CREATE_DISPATCH(hs_error_t, hs_open_stream, const hs_database_t *db,
                unsigned int flags, hs_stream_t **stream);
CONNECT_ARGS_1(hs_error_t, hs_open_stream, db, flags, stream);
CONNECT_DISPATCH_2(hs_error_t, hs_open_stream, const hs_database_t *db,
                unsigned int flags, hs_stream_t **stream);
CONNECT_ARGS_3(hs_error_t, hs_open_stream, db, flags, stream);

CREATE_DISPATCH(hs_error_t, hs_scan_stream, hs_stream_t *id, const char *data,
                unsigned int length, unsigned int flags, hs_scratch_t *scratch,
                match_event_handler onEvent, void *ctxt);
CONNECT_ARGS_1(hs_error_t, hs_scan_stream, id, data, length, flags, scratch, onEvent, ctxt);
CONNECT_DISPATCH_2(hs_error_t, hs_scan_stream, hs_stream_t *id, const char *data,
                unsigned int length, unsigned int flags, hs_scratch_t *scratch,
                match_event_handler onEvent, void *ctxt);
CONNECT_ARGS_3(hs_error_t, hs_scan_stream, id, data, length, flags, scratch, onEvent, ctxt);

CREATE_DISPATCH(hs_error_t, hs_close_stream, hs_stream_t *id,
                hs_scratch_t *scratch, match_event_handler onEvent, void *ctxt);
CONNECT_ARGS_1(hs_error_t, hs_close_stream, id, scratch, onEvent, ctxt);
CONNECT_DISPATCH_2(hs_error_t, hs_close_stream, hs_stream_t *id,
                hs_scratch_t *scratch, match_event_handler onEvent, void *ctxt);
CONNECT_ARGS_3(hs_error_t, hs_close_stream, id, scratch, onEvent, ctxt);

CREATE_DISPATCH(hs_error_t, hs_scan_vector, const hs_database_t *db,
                const char *const *data, const unsigned int *length,
                unsigned int count, unsigned int flags, hs_scratch_t *scratch,
                match_event_handler onevent, void *context);
CONNECT_ARGS_1(hs_error_t, hs_scan_vector, db, data, length, count, flags, scratch, onevent, context);
CONNECT_DISPATCH_2(hs_error_t, hs_scan_vector, const hs_database_t *db,
                const char *const *data, const unsigned int *length,
                unsigned int count, unsigned int flags, hs_scratch_t *scratch,
                match_event_handler onevent, void *context);
CONNECT_ARGS_3(hs_error_t, hs_scan_vector, db, data, length, count, flags, scratch, onevent, context);

CREATE_DISPATCH(hs_error_t, hs_database_info, const hs_database_t *db, char **info);
CONNECT_ARGS_1(hs_error_t, hs_database_info, db, info);
CONNECT_DISPATCH_2(hs_error_t, hs_database_info, const hs_database_t *db, char **info);
CONNECT_ARGS_3(hs_error_t, hs_database_info, db, info);

CREATE_DISPATCH(hs_error_t, hs_copy_stream, hs_stream_t **to_id,
                const hs_stream_t *from_id);
CONNECT_ARGS_1(hs_error_t, hs_copy_stream, to_id, from_id);
CONNECT_DISPATCH_2(hs_error_t, hs_copy_stream, hs_stream_t **to_id,
                const hs_stream_t *from_id);
CONNECT_ARGS_3(hs_error_t, hs_copy_stream, to_id, from_id);

CREATE_DISPATCH(hs_error_t, hs_reset_stream, hs_stream_t *id,
                unsigned int flags, hs_scratch_t *scratch,
                match_event_handler onEvent, void *context);
CONNECT_ARGS_1(hs_error_t, hs_reset_stream, id, flags, scratch, onEvent, context);
CONNECT_DISPATCH_2(hs_error_t, hs_reset_stream, hs_stream_t *id,
                unsigned int flags, hs_scratch_t *scratch,
                match_event_handler onEvent, void *context);
CONNECT_ARGS_3(hs_error_t, hs_reset_stream, id, flags, scratch, onEvent, context);

CREATE_DISPATCH(hs_error_t, hs_reset_and_copy_stream, hs_stream_t *to_id,
                const hs_stream_t *from_id, hs_scratch_t *scratch,
                match_event_handler onEvent, void *context);
CONNECT_ARGS_1(hs_error_t, hs_reset_and_copy_stream, to_id, from_id, scratch, onEvent, context);
CONNECT_DISPATCH_2(hs_error_t, hs_reset_and_copy_stream, hs_stream_t *to_id,
                const hs_stream_t *from_id, hs_scratch_t *scratch,
                match_event_handler onEvent, void *context);
CONNECT_ARGS_3(hs_error_t, hs_reset_and_copy_stream, to_id, from_id, scratch, onEvent, context);

CREATE_DISPATCH(hs_error_t, hs_serialize_database, const hs_database_t *db,
                char **bytes, size_t *length);
CONNECT_ARGS_1(hs_error_t, hs_serialize_database, db, bytes, length);
CONNECT_DISPATCH_2(hs_error_t, hs_serialize_database, const hs_database_t *db,
                char **bytes, size_t *length);
CONNECT_ARGS_3(hs_error_t, hs_serialize_database, db, bytes, length);

CREATE_DISPATCH(hs_error_t, hs_deserialize_database, const char *bytes,
                const size_t length, hs_database_t **db);
CONNECT_ARGS_1(hs_error_t, hs_deserialize_database, bytes, length, db);
CONNECT_DISPATCH_2(hs_error_t, hs_deserialize_database, const char *bytes,
                const size_t length, hs_database_t **db);
CONNECT_ARGS_3(hs_error_t, hs_deserialize_database, bytes, length, db);

CREATE_DISPATCH(hs_error_t, hs_deserialize_database_at, const char *bytes,
                const size_t length, hs_database_t *db);
CONNECT_ARGS_1(hs_error_t, hs_deserialize_database_at, bytes, length, db);
CONNECT_DISPATCH_2(hs_error_t, hs_deserialize_database_at, const char *bytes,
                const size_t length, hs_database_t *db);
CONNECT_ARGS_3(hs_error_t, hs_deserialize_database_at, bytes, length, db);

CREATE_DISPATCH(hs_error_t, hs_serialized_database_info, const char *bytes,
                size_t length, char **info);
CONNECT_ARGS_1(hs_error_t, hs_serialized_database_info, bytes, length, info);
CONNECT_DISPATCH_2(hs_error_t, hs_serialized_database_info, const char *bytes,
                size_t length, char **info);
CONNECT_ARGS_3(hs_error_t, hs_serialized_database_info, bytes, length, info);

CREATE_DISPATCH(hs_error_t, hs_serialized_database_size, const char *bytes,
                const size_t length, size_t *deserialized_size);
CONNECT_ARGS_1(hs_error_t, hs_serialized_database_size, bytes, length, deserialized_size);
CONNECT_DISPATCH_2(hs_error_t, hs_serialized_database_size, const char *bytes,
                const size_t length, size_t *deserialized_size);
CONNECT_ARGS_3(hs_error_t, hs_serialized_database_size, bytes, length, deserialized_size);

CREATE_DISPATCH(hs_error_t, hs_compress_stream, const hs_stream_t *stream,
                char *buf, size_t buf_space, size_t *used_space);
CONNECT_ARGS_1(hs_error_t, hs_compress_stream, stream,
                buf, buf_space, used_space);
CONNECT_DISPATCH_2(hs_error_t, hs_compress_stream, const hs_stream_t *stream,
                char *buf, size_t buf_space, size_t *used_space);
CONNECT_ARGS_3(hs_error_t, hs_compress_stream, stream,
                buf, buf_space, used_space);

CREATE_DISPATCH(hs_error_t, hs_expand_stream, const hs_database_t *db,
                hs_stream_t **stream, const char *buf,size_t buf_size);
CONNECT_ARGS_1(hs_error_t, hs_expand_stream, db, stream, buf,buf_size);
CONNECT_DISPATCH_2(hs_error_t, hs_expand_stream, const hs_database_t *db,
                hs_stream_t **stream, const char *buf,size_t buf_size);
CONNECT_ARGS_3(hs_error_t, hs_expand_stream, db, stream, buf,buf_size);

CREATE_DISPATCH(hs_error_t, hs_reset_and_expand_stream, hs_stream_t *to_stream,
                const char *buf, size_t buf_size, hs_scratch_t *scratch,
                match_event_handler onEvent, void *context);
CONNECT_ARGS_1(hs_error_t, hs_reset_and_expand_stream, to_stream,
                buf, buf_size, scratch, onEvent, context);
CONNECT_DISPATCH_2(hs_error_t, hs_reset_and_expand_stream, hs_stream_t *to_stream,
                const char *buf, size_t buf_size, hs_scratch_t *scratch,
                match_event_handler onEvent, void *context);
CONNECT_ARGS_3(hs_error_t, hs_reset_and_expand_stream, to_stream,
                buf, buf_size, scratch, onEvent, context);

/** INTERNALS **/

CREATE_DISPATCH(u32, Crc32c_ComputeBuf, u32 inCrc32, const void *buf, size_t bufLen);
CONNECT_ARGS_1(u32, Crc32c_ComputeBuf, inCrc32, buf, bufLen);
CONNECT_DISPATCH_2(u32, Crc32c_ComputeBuf, u32 inCrc32, const void *buf, size_t bufLen);
CONNECT_ARGS_3(u32, Crc32c_ComputeBuf, inCrc32, buf, bufLen);

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop

