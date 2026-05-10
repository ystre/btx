#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# shellcheck source=../common.sh
. "${SCRIPT_DIR}/../common.sh"

TMPDIR="$(mktemp -d)"
trap 'rm -rf "${TMPDIR}"' EXIT

run() { "${BTX_BIN}" "$@" 2>&1; echo "EXIT:$?"; }
exit_code() { "${BTX_BIN}" "$@" >/dev/null 2>&1 && echo 0 || echo $?; }

# --help and --version exit 0
assert_exit 0 "$(exit_code --help)"    "--help exits 0"
assert_exit 0 "$(exit_code --version)" "--version exits 0"
assert_exit 0 "$(exit_code -h)"        "-h exits 0"
assert_exit 0 "$(exit_code -V)"        "-V exits 0"

# --version output
out="$("${BTX_BIN}" --version)"
assert_output_contains "btx" "${out}" "--version contains 'btx'"

# --help output
out="$("${BTX_BIN}" --help)"
assert_output_contains "-r" "${out}" "--help mentions -r"

# No arguments exits non-zero
assert_exit 1 "$(exit_code)" "no args exits 1"

# Non-existent file
assert_exit 1 "$(exit_code /nonexistent/file.bin)" "non-existent file exits 1"
assert_exit 1 "$(exit_code -r /nonexistent/file.btx)" "non-existent file with -r exits 1"

# Invalid BTX input for decode
printf 'not valid btx' > "${TMPDIR}/bad.btx"
assert_exit 1 "$(exit_code -r "${TMPDIR}/bad.btx")" "decode invalid input exits 1"

summary
