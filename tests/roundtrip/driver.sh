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
    "${BTX_BIN}" "${TMPDIR}/original.bin" > "${TMPDIR}/encoded.btx"
    "${BTX_BIN}" -r "${TMPDIR}/encoded.btx" > "${TMPDIR}/decoded.bin"
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
"${BTX_BIN}" "${TMPDIR}/all.bin" > "${TMPDIR}/all.btx"
"${BTX_BIN}" -r "${TMPDIR}/all.btx" > "${TMPDIR}/all_decoded.bin"
if cmp -s "${TMPDIR}/all.bin" "${TMPDIR}/all_decoded.bin"; then
    pass "roundtrip all 256 bytes"
else
    fail "roundtrip all 256 bytes: mismatch"
fi

# Empty file
printf '' > "${TMPDIR}/empty.bin"
"${BTX_BIN}" "${TMPDIR}/empty.bin" > "${TMPDIR}/empty.btx"
"${BTX_BIN}" -r "${TMPDIR}/empty.btx" > "${TMPDIR}/empty_decoded.bin"
if cmp -s "${TMPDIR}/empty.bin" "${TMPDIR}/empty_decoded.bin"; then
    pass "roundtrip empty file"
else
    fail "roundtrip empty file: mismatch"
fi

summary
