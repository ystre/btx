#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck source=../common.sh
. "${SCRIPT_DIR}/../common.sh"

TMPDIR="$(mktemp -d)"
trap 'rm -rf "${TMPDIR}"' EXIT

btx_decode() {
    printf '%s' "$1" > "${TMPDIR}/input.btx"
    "${BTX_BIN}" -r "${TMPDIR}/input.btx" | xxd -p
}

# Single hex token
assert_output 'ab' "$(btx_decode '\xab')" "decode single hex token"

# Multiple hex tokens
assert_output '00ff12ac' "$(btx_decode '\x00\xff\x12\xac')" "decode multiple hex tokens"

# Case insensitive
assert_output 'ab' "$(btx_decode '\xAB')" "decode uppercase hex"
assert_output 'ab' "$(btx_decode '\xAb')" "decode mixed case hex"

# Whitespace and comments ignored
assert_output 'ab' "$(btx_decode '  \xab  ')" "decode with surrounding whitespace"
assert_output 'ab' "$(btx_decode $'// comment\n\\xab')" "decode with comment"
assert_output 'abcd' "$(btx_decode $'\\xab // inline\n\\xcd')" "decode with inline comment"

# Bit tokens
assert_output 'f0' "$(btx_decode '\b11110000')" "decode full bit token"
assert_output 'f0' "$(btx_decode "\b1111'0000")" "decode bit token with separator"
assert_output 'f0' "$(btx_decode "\b1111'____ \b____'0000")" "decode two partial bit tokens"

# Empty input
printf '' > "${TMPDIR}/empty.btx"
out="$("${BTX_BIN}" -r "${TMPDIR}/empty.btx" | xxd -p)"
assert_output '' "${out}" "decode empty input"

# stdin via -
out="$(printf '\\xab' | "${BTX_BIN}" -r - | xxd -p)"
assert_output 'ab' "${out}" "decode from stdin"

summary
