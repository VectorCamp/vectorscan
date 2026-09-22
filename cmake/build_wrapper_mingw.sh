#!/bin/sh -e
# This is used for renaming symbols for the fat runtime, don't call directly
# TODO: make this a lot less fragile!
cleanup () {
    rm -f ${SYMSFILE} ${KEEPSYMS}
}

NM="${NM:-nm}"
OBJCOPY="${OBJCOPY:-objcopy}"
OBJDUMP="${OBJDUMP:-objdump}"

PREFIX=$1
KEEPSYMS_IN=$2
shift 2
# $@ contains the actual build command
OUT=$(echo "$@" | rev | cut -d ' ' -f 2- | rev | sed 's/.* -o \(.*\.obj\).*/\1/')
trap cleanup INT QUIT EXIT
SYMSFILE=$(mktemp -p /tmp ${PREFIX}_rename.syms.XXXXX)
KEEPSYMS=$(mktemp -p /tmp keep.syms.XXXXX)
# Use UCRT64 libcrtbase.a instead of libc.so.6
LIBC_SO=$("$@" --print-file-name=libucrtbase.a)

NM_FLAG="-f"
cp ${KEEPSYMS_IN} ${KEEPSYMS}
# get all symbols from libc and turn them into patterns (no -D flag for static libs)
${NM} ${NM_FLAG} posix -g ${LIBC_SO} | sed 's/\([^ @]*\).*/^\1$/' >> ${KEEPSYMS}
# build the object
"$@"
# rename the symbols in the object
${NM} ${NM_FLAG} posix -g ${OUT} | cut -f1 -d' ' | grep -v -f ${KEEPSYMS} | sed "s/\(.*\)/\1 ${PREFIX}_\1/" >> ${SYMSFILE}
if test -s ${SYMSFILE}
then
    ${OBJCOPY} --redefine-syms=${SYMSFILE} ${OUT}
fi

# Also rename .refptr sections to avoid COMDAT conflicts (MinGW-specific)
SECTFILE=$(mktemp -p /tmp ${PREFIX}_sections.XXXXX)

# Get list of .rdata$.refptr.* sections - use objdump -h to show section headers
${OBJDUMP} -h ${OUT} 2>/dev/null | grep '\.rdata\$\.refptr\.' | awk '{print $2}' | while IFS= read -r sect; do
    # Skip mmbit sections (shared lookup tables across all variants)
    case "$sect" in
        *.refptr.mmbit*|*.refptr.hs_*) continue ;;
    esac
    echo "--rename-section=${sect}=${PREFIX}_${sect}" >> ${SECTFILE}
done 2>/dev/null

if [ -s "${SECTFILE}" ]; then
    ${OBJCOPY} $(cat ${SECTFILE}) ${OUT} 2>/dev/null
fi
rm -f "${SECTFILE}" 2>/dev/null
