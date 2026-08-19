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
requirements=$(printf '%s
' "$requires" | tr ',' '
' | awk '{$1=$1; if (NF) print}')
requirement_count=$(printf '%s
' "$requirements" | awk 'NF { count += 1 } END { print count + 0 }')
[ "$requirement_count" -eq 3 ] || fail 'public dependency closure must contain exactly three version clauses'
count_requirement() {
  printf '%s
' "$requirements" | awk \
    -v package="$1" -v operator="$2" -v version="$3" '
      $1 == package && $2 == operator && $3 == version { count += 1 }
      END { print count + 0 }
    '
}
[ "$(count_requirement libpkgreconcile '>=' 0.3.0)" -eq 1 ] || fail 'expected exactly one libpkgreconcile >= 0.3.0 requirement'
[ "$(count_requirement libpkgapply '>=' 4.0.0)" -eq 1 ] || fail 'expected exactly one libpkgapply >= 4.0.0 requirement'
[ "$(count_requirement libpkgapply '<' 5.0.0)" -eq 1 ] || fail 'expected exactly one libpkgapply < 5.0.0 requirement'
if printf '%s
' "$requirements" | awk '$1 != "libpkgreconcile" && $1 != "libpkgapply" { bad = 1 } END { exit bad ? 0 : 1 }'; then
  fail 'unexpected public dependency leaked into pkg-config'
fi
if grep -E 'libpkgapply-posix|libpkgreconcile-posix|libpkgstate|pkgctl' "$pc" >/dev/null; then
  fail 'neighboring mechanism/state/controller dependency leaked into pkg-config'
fi
printf '%s
' 'pkgconfig-metadata-contract: ok'
