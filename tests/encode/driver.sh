#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck source=../common.sh
. "${SCRIPT_DIR}/../common.sh"

TMPDIR="$(mktemp -d)"
trap 'rm -rf "${TMPDIR}"' EXIT

# Single byte
printf '\xab' > "${TMPDIR}/input.bin"
out="$("${BTX_BIN}" "${TMPDIR}/input.bin")"
assert_output '\xab' "${out}" "encode single byte"

# Multiple bytes
printf '\x00\xff\x12\xac' > "${TMPDIR}/input.bin"
out="$("${BTX_BIN}" "${TMPDIR}/input.bin")"
assert_output '\x00\xff\x12\xac' "${out}" "encode multiple bytes"

# All 256 bytes produce 1024 chars output
python3 -c 'import sys; sys.stdout.buffer.write(bytes(range(256)))' > "${TMPDIR}/all.bin"
out="$("${BTX_BIN}" "${TMPDIR}/all.bin")"
assert_output "1024" "${#out}" "encode all bytes length"

# Empty input
printf '' > "${TMPDIR}/empty.bin"
out="$("${BTX_BIN}" "${TMPDIR}/empty.bin")"
assert_output '' "${out}" "encode empty input"

# stdin via -
out="$(printf '\xcd' | "${BTX_BIN}" -)"
assert_output '\xcd' "${out}" "encode from stdin"

summary
