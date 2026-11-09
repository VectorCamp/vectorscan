#!/bin/sh -e
# This is used for renaming symbols for the fat runtime, don't call directly
# TODO: make this a lot less fragile!
cleanup () {
    rm -f ${SYMSFILE} ${KEEPSYMS}
}

#set -x

NM="${NM:-nm}"
OBJCOPY="${OBJCOPY:-objcopy}"
OBJDUMP="${OBJDUMP:-objdump}"
if command -v ccache >/dev/null 2>&1; then
    CCACHE=ccache
else
    CCACHE=
fi

PREFIX=$1
KEEPSYMS_IN=$2
LIBCSYMS=$3

shift 3
# $@ contains the actual build command
# The output object is the argument following the last -o.
OUT=
_prev=
for _arg in "$@"; do
    if [ "${_prev}" = "-o" ]; then
        OUT=${_arg}
    fi
    _prev=${_arg}
done
if [ -z "${OUT}" ]; then
    echo "$0: could not determine the output object from the build command" >&2
    exit 1
fi
trap cleanup INT QUIT EXIT
SYMSFILE=$(mktemp -p /tmp ${PREFIX}_rename.syms.XXXXX)
KEEPSYMS=$(mktemp -p /tmp keep.syms.XXXXX)

NM_FLAG="-f"
cat ${KEEPSYMS_IN} ${LIBCSYMS} >> ${KEEPSYMS}

# build the object
${CCACHE} "$@"
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
    case "$sect" in
        *.refptr.mmbit*|*.refptr.hs_*|*.refptr.rose*) continue ;;
    esac
    echo "--rename-section=${sect}=${PREFIX}_${sect}" >> ${SECTFILE}
done 2>/dev/null

if [ -s "${SECTFILE}" ]; then
    ${OBJCOPY} $(cat ${SECTFILE}) ${OUT} 2>/dev/null
fi
rm -f "${SECTFILE}" 2>/dev/null
