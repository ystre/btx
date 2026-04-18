#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck source=../common.sh
. "${SCRIPT_DIR}/../common.sh"

TMPDIR="$(mktemp -d)"
trap 'rm -rf "${TMPDIR}"' EXIT

btx_validate() {
    printf '%s' "$1" > "${TMPDIR}/input.btx"
    "${BTX_BIN}" validate "${TMPDIR}/input.btx"; echo $?
}

assert_valid() {
    local input="$1" context="$2"
    printf '%s' "${input}" > "${TMPDIR}/input.btx"
    "${BTX_BIN}" validate "${TMPDIR}/input.btx"
    assert_exit 0 $? "${context}"
}

assert_invalid() {
    local input="$1" context="$2"
    printf '%s' "${input}" > "${TMPDIR}/input.btx"
    "${BTX_BIN}" validate "${TMPDIR}/input.btx" && rc=0 || rc=$?
    assert_exit 1 "${rc}" "${context}"
}

# Valid inputs
assert_valid '\xab' "valid single hex token"
assert_valid '\x00\xff' "valid multiple hex tokens"
assert_valid '\xAB\xCd' "valid mixed case hex"
assert_valid $'// comment\n\\xab' "valid with comment"
assert_valid $'\\xab // inline\n\\xcd' "valid with inline comment"
assert_valid '' "valid empty input"
assert_valid $'   \t  \n  ' "valid whitespace only"
assert_valid '\b11110000' "valid full bit token"
assert_valid "\b1111'0000" "valid bit token with separator"
assert_valid "\b1111'____ \b____'0000" "valid two partial bit tokens"
assert_valid '\b11______ \b__1100__ \b______11' "valid three partial bit tokens"

# Invalid inputs
assert_invalid '\xGG' "invalid hex digits"
assert_invalid 'hello' "invalid token"
assert_invalid '\x' "truncated hex token"
assert_invalid '\b1111' "truncated bit token"
assert_invalid "\b1__1'____" "non-contiguous bit ownership"
assert_invalid "\b1111'____ \b______11" "out of order bit ownership"
assert_invalid "\b1_______ \b1_______" "overlapping bit ownership"
assert_invalid "\b1111'____" "incomplete byte at EOF"
assert_invalid "\b1111'____ \xff" "incomplete byte before hex token"
assert_invalid "\b11110000'" "trailing apostrophe"

summary
