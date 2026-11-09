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
LIBCSYMS=$3
if command -v ccache >/dev/null 2>&1; then
    CCACHE=ccache
else
    CCACHE=
fi

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

# get all symbols from libc and turn them into pattern
${NM} ${NM_FLAG} posix -g ${OUT} | cut -f1 -d' ' | grep -v -f ${KEEPSYMS} | sed -e "s/\(.*\)/\1\ ${PREFIX}_\1/" >> ${SYMSFILE}
if test -s ${SYMSFILE}
then
    ${OBJCOPY} --redefine-syms=${SYMSFILE} ${OUT}
fi
