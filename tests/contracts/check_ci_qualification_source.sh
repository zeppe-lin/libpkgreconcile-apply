#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=$1
workflow=$root/.github/workflows/ci.yml
fail() { echo "ci-qualification-source-contract: $*" >&2; exit 1; }
[ -s "$workflow" ] || fail 'CI workflow is missing'
grep -F 'for suite in unit integration header contract' "$root/ci/configure-and-test.sh" >/dev/null ||
  fail 'CI suite loop does not enumerate unit/integration/header/contract explicitly'
grep -F -- '--suite "$suite"' "$root/ci/configure-and-test.sh" >/dev/null ||
  fail 'CI suite loop does not execute each named suite'
for mode in shared static; do
  grep -F "mode: $mode" "$workflow" >/dev/null || fail "CI does not qualify $mode linkage"
done
for compiler in 'cxx: g++' 'cxx: clang++'; do
  grep -F "$compiler" "$workflow" >/dev/null || fail "CI does not qualify $compiler"
done
grep -F 'b_sanitize=address,undefined' "$workflow" >/dev/null || fail 'CI does not qualify ASan+UBSan'
grep -F 'doxygen Doxyfile' "$workflow" >/dev/null || fail 'CI does not validate Doxygen'
grep -F 'ref: v0.3.0' "$workflow" >/dev/null || fail 'CI does not pin libpkgreconcile 0.3.0'
printf '%s\n' 'ci-qualification-source-contract: ok'
