#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(git -C "${SCRIPT_DIR}" rev-parse --show-toplevel)"
BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build/debug}"
BTX_BIN="${BTX_BIN:-${BUILD_DIR}/btx-cli/btx}"

if [[ ! -x "${BTX_BIN}" ]]; then
    echo "btx binary not found or not executable: ${BTX_BIN}" >&2
    exit 1
fi

PASS=0
FAIL=0

pass() { echo "PASS: $*"; PASS=$((PASS + 1)); }
fail() { echo "FAIL: $*" >&2; FAIL=$((FAIL + 1)); }

summary() {
    echo "${PASS} passed, ${FAIL} failed"
    [[ ${FAIL} -eq 0 ]]
}

assert_exit() {
    local expected="$1" actual="$2" context="$3"
    if [[ "${expected}" -eq "${actual}" ]]; then
        pass "${context}"
    else
        fail "${context}: expected exit ${expected}, got ${actual}"
    fi
}

assert_output() {
    local expected="$1" actual="$2" context="$3"
    if [[ "${expected}" == "${actual}" ]]; then
        pass "${context}"
    else
        fail "${context}: expected '${expected}', got '${actual}'"
    fi
}

assert_output_contains() {
    local needle="$1" actual="$2" context="$3"
    if echo "${actual}" | grep -qF "${needle}"; then
        pass "${context}"
    else
        fail "${context}: expected '${needle}' in output '${actual}'"
    fi
}
