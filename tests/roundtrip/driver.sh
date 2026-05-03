#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck source=../common.sh
. "${SCRIPT_DIR}/../common.sh"

TMPDIR="$(mktemp -d)"
trap 'rm -rf "${TMPDIR}"' EXIT

roundtrip() {
    local input_hex="$1" context="$2"
    printf "${input_hex}" > "${TMPDIR}/original.bin"
    "${BTX_BIN}" encode "${TMPDIR}/original.bin" > "${TMPDIR}/encoded.btx"
    "${BTX_BIN}" decode "${TMPDIR}/encoded.btx" > "${TMPDIR}/decoded.bin"
    if cmp -s "${TMPDIR}/original.bin" "${TMPDIR}/decoded.bin"; then
        pass "${context}"
    else
        fail "${context}: roundtrip mismatch"
    fi
}

roundtrip '\x00' "roundtrip single zero byte"
roundtrip '\xff' "roundtrip single 0xff byte"
roundtrip '\x00\xff\x12\xac' "roundtrip multiple bytes"

# All 256 bytes
python3 -c 'import sys; sys.stdout.buffer.write(bytes(range(256)))' > "${TMPDIR}/all.bin"
"${BTX_BIN}" encode "${TMPDIR}/all.bin" > "${TMPDIR}/all.btx"
"${BTX_BIN}" decode "${TMPDIR}/all.btx" > "${TMPDIR}/all_decoded.bin"
if cmp -s "${TMPDIR}/all.bin" "${TMPDIR}/all_decoded.bin"; then
    pass "roundtrip all 256 bytes"
else
    fail "roundtrip all 256 bytes: mismatch"
fi

# Empty file
printf '' > "${TMPDIR}/empty.bin"
"${BTX_BIN}" encode "${TMPDIR}/empty.bin" > "${TMPDIR}/empty.btx"
"${BTX_BIN}" decode "${TMPDIR}/empty.btx" > "${TMPDIR}/empty_decoded.bin"
if cmp -s "${TMPDIR}/empty.bin" "${TMPDIR}/empty_decoded.bin"; then
    pass "roundtrip empty file"
else
    fail "roundtrip empty file: mismatch"
fi

# Encoded output is valid BTX
python3 -c 'import sys; sys.stdout.buffer.write(bytes(range(256)))' > "${TMPDIR}/all.bin"
"${BTX_BIN}" encode "${TMPDIR}/all.bin" > "${TMPDIR}/all.btx"
"${BTX_BIN}" validate "${TMPDIR}/all.btx"
assert_exit 0 $? "encoded output passes validate"

summary
