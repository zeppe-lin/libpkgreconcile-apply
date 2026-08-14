#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
build=$1
expected_version=$2
pc=$build/meson-private/libpkgreconcile-apply.pc
fail() { echo "pkgconfig-metadata-contract: $*" >&2; [ ! -f "$pc" ] || cat "$pc" >&2; exit 1; }
[ -s "$pc" ] || fail 'generated libpkgreconcile-apply.pc is missing'
version=$(sed -n 's/^Version:[[:space:]]*//p' "$pc")
[ "$version" = "$expected_version" ] || fail "version is '$version', expected '$expected_version'"
requires=$(sed -n 's/^Requires:[[:space:]]*//p' "$pc")
case $requires in
  *'libpkgreconcile >= 0.3.0'*'libpkgapply >= 3.0.1'*) ;;
  *) fail 'public dependency closure is not libpkgreconcile + libpkgapply' ;;
esac
if grep -E 'libpkgapply-posix|libpkgreconcile-posix|libpkgstate|pkgctl' "$pc" >/dev/null; then
  fail 'neighboring mechanism/state/controller dependency leaked into pkg-config'
fi
printf '%s\n' 'pkgconfig-metadata-contract: ok'
