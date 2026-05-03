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

# --help output contains commands
out="$("${BTX_BIN}" --help)"
assert_output_contains "encode" "${out}" "--help mentions encode"
assert_output_contains "decode" "${out}" "--help mentions decode"
assert_output_contains "validate" "${out}" "--help mentions validate"

# No arguments exits non-zero
assert_exit 1 "$(exit_code)" "no args exits 1"

# Unknown command exits non-zero
assert_exit 1 "$(exit_code foobar)" "unknown command exits 1"
out="$(run foobar)"
assert_output_contains "foobar" "${out}" "unknown command error mentions command name"

# Missing file argument for each command
for cmd in encode decode validate; do
    assert_exit 1 "$(exit_code "${cmd}")" "${cmd} without file arg exits 1"
    out="$(run "${cmd}")"
    assert_output_contains "${cmd}" "${out}" "${cmd} without file arg error mentions command"
done

# Extra argument
printf '\xab' > "${TMPDIR}/f.bin"
assert_exit 1 "$(exit_code encode "${TMPDIR}/f.bin" extra)" "extra argument exits 1"

# Non-existent file
assert_exit 1 "$(exit_code encode /nonexistent/file.bin)" "non-existent file exits 1"

# Invalid BTX input for decode
printf 'not valid btx' > "${TMPDIR}/bad.btx"
assert_exit 1 "$(exit_code decode "${TMPDIR}/bad.btx")" "decode invalid input exits 1"

# Invalid BTX input for validate
assert_exit 1 "$(exit_code validate "${TMPDIR}/bad.btx")" "validate invalid input exits 1"

summary
