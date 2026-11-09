#!/bin/sh -e

NM="${NM:-nm}"

CC=$1
LIBC=$2
LIBCSYMS=$3
EXTRA_NM_FLAG=${4:-}

# Find location of C Library
LIBC_SO=$(${CC} --print-file-name=${LIBC})

NM_FLAG="-f"
# get all symbols from libc and turn them into patterns (no -D flag for static libs)
${NM} ${NM_FLAG} posix ${EXTRA_NM_FLAG} -g ${LIBC_SO} | sed 's/\([^ @]*\).*/^\1$/' > ${LIBCSYMS}

