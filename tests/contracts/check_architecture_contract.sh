#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=$1
fail() { echo "architecture-contract: $*" >&2; exit 1; }

# Product code is a pure composition adapter. Only the semantic application and
# reconciliation owners may cross this boundary.
if grep -RInE '#[[:space:]]*include[[:space:]]*<(libpkgapply-posix|libpkgreconcile-posix|libpkgstate|pkgctl|unistd\.h|fcntl\.h|sys/)' \
    "$root/include" "$root/src" --include='*.h' --include='*.cpp' >/dev/null; then
  fail 'product imports mechanism, state, controller, or POSIX authority'
fi

actual=$(sed -n "s/^\([A-Za-z0-9_]*_dep\)[[:space:]]*=[[:space:]]*dependency('\([^']*\)'.*/\2/p" "$root/meson.build" | sort -u)
expected='libpkgapply
libpkgreconcile'
[ "$actual" = "$expected" ] || {
  printf '%s\n' "$actual" >&2
  fail 'product dependency boundary is not exactly libpkgapply + libpkgreconcile'
}

if grep -RInE '/var/lib/pkg/rejected|rejected-v[0-9]|generations/|renameat2|flock\(' \
    "$root/include" "$root/src" >/dev/null; then
  fail 'product contains provider-private persistence/layout knowledge'
fi

printf '%s\n' 'architecture-contract: ok'
