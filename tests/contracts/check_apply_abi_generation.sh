#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
library=${1:?}
fail() { echo "apply-abi-generation-contract: $*" >&2; exit 1; }
command -v readelf >/dev/null 2>&1 || fail 'readelf is required'
needed=$(readelf -d "$library" | sed -n 's/^.*Shared library: \[\(.*\)\].*$/\1/p')
printf '%s\n' "$needed" | grep -Fx 'libpkgapply.so.4' >/dev/null ||
  fail 'shared product is not bound to libpkgapply.so.4'
if printf '%s\n' "$needed" | grep -E '^libpkgapply\.so\.[123]$' >/dev/null; then
  fail 'obsolete libpkgapply ABI generation remains in the shared closure'
fi
