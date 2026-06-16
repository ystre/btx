#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck source=../common.sh
. "${SCRIPT_DIR}/../common.sh"

TMPDIR="$(mktemp -d)"
trap 'rm -rf "${TMPDIR}"' EXIT

# Setup: create a test binary file
printf '\x01\x02\x03' > "${TMPDIR}/input.bin"

# No args: reads stdin, writes stdout
out="$(printf '\x01\x02\x03' | "${BTX_BIN}")"
assert_output '\x01\x02\x03' "${out}" "no args reads stdin"

# Infile only: reads file, writes stdout
out="$("${BTX_BIN}" "${TMPDIR}/input.bin")"
assert_output '\x01\x02\x03' "${out}" "infile only writes stdout"

# Infile + outfile: reads file, writes to outfile
"${BTX_BIN}" "${TMPDIR}/input.bin" "${TMPDIR}/output.btx"
out="$(cat "${TMPDIR}/output.btx")"
assert_output '\x01\x02\x03' "${out}" "infile + outfile writes file"

# Stdin via -: reads stdin, writes stdout
out="$(printf '\x01\x02\x03' | "${BTX_BIN}" -)"
assert_output '\x01\x02\x03' "${out}" "- as infile reads stdin"

# Stdin + outfile: reads stdin, writes to outfile
printf '\x01\x02\x03' | "${BTX_BIN}" - "${TMPDIR}/output2.btx"
out="$(cat "${TMPDIR}/output2.btx")"
assert_output '\x01\x02\x03' "${out}" "- infile + outfile writes file"

# Outfile as -: writes stdout explicitly
out="$("${BTX_BIN}" "${TMPDIR}/input.bin" -)"
assert_output '\x01\x02\x03' "${out}" "- as outfile writes stdout"

# Reverse with positionals
"${BTX_BIN}" "${TMPDIR}/input.bin" "${TMPDIR}/encoded.btx"
"${BTX_BIN}" -r "${TMPDIR}/encoded.btx" "${TMPDIR}/decoded.bin"
assert_output "$(xxd -p "${TMPDIR}/input.bin")" "$(xxd -p "${TMPDIR}/decoded.bin")" "reverse with infile + outfile"

summary
