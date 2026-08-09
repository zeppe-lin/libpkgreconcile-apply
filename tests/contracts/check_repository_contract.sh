#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=$1
fail() { echo "repository-contract: $*" >&2; exit 1; }
for required in \
  include/libpkgreconcile-apply/export.h \
  include/libpkgreconcile-apply/adapter.h \
  include/libpkgreconcile-apply/libpkgreconcile-apply.h \
  src/adapter.cpp src/meson.build \
  abi/libpkgreconcile-apply.exports \
  scripts/generate-elf-export-script.sh \
  tests/meson.build; do
  [ -s "$root/$required" ] || fail "missing or empty $required"
done
for directory in tests/unit tests/integration tests/header tests/support tests/contracts; do
  [ -d "$root/$directory" ] || fail "missing $directory"
done
for forbidden in tools protocol mechanism; do
  [ ! -e "$root/$forbidden" ] || fail "unexpected repository surface: $forbidden"
done
printf '%s\n' 'repository-contract: ok'
