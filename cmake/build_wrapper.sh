#!/bin/sh -e
# This is used for renaming symbols for the fat runtime, don't call directly
# TODO: make this a lot less fragile!
cleanup () {
    rm -f ${SYMSFILE} ${KEEPSYMS}
}

NM="${NM:-nm}"
OBJCOPY="${OBJCOPY:-objcopy}"

PREFIX=$1
KEEPSYMS_IN=$2
shift 2
# $@ contains the actual build command
OUT=$(echo "$@" | rev | cut -d ' ' -f 2- | rev | sed 's/.* -o \(.*\.o\).*/\1/')
trap cleanup INT QUIT EXIT
SYMSFILE=$(mktemp -p /tmp ${PREFIX}_rename.syms.XXXXX)
KEEPSYMS=$(mktemp -p /tmp keep.syms.XXXXX)
# find the libc used by gcc
LIBC_SO=$("$@" --print-file-name=libc.so.6)
NM_FLAG="-f"
if [ `uname` = "FreeBSD" ]; then
    # for freebsd, we will specify the name, 
    # we will leave it work as is in linux
    LIBC_SO=/lib/libc.so.7
    # also, in BSD, the nm flag -F corresponds to the -f flag in linux.
    NM_FLAG="-F"
fi
cp ${KEEPSYMS_IN} ${KEEPSYMS}
# get all symbols from libc and turn them into patterns
${NM} ${NM_FLAG} posix -g -D ${LIBC_SO} | sed 's/\([^ @]*\).*/^\1$/' >> ${KEEPSYMS}
# build the object
"$@"
# rename the symbols in the object
${NM} ${NM_FLAG} posix -g ${OUT} | cut -f1 -d' ' | grep -v -f ${KEEPSYMS} | sed -e "s/\(.*\)/\1\ ${PREFIX}_\1/" >> ${SYMSFILE}
if test -s ${SYMSFILE}
then
    ${OBJCOPY} --redefine-syms=${SYMSFILE} ${OUT}
fi
