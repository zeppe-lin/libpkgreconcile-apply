#!/bin/sh
# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
set -eu
root=$1
fail() { echo "documentation-source-contract: $*" >&2; exit 1; }

for required in README.md DESIGN.md HISTORY.md CONTRIBUTING.md MAINTAINING.md TESTING.md man/libpkgreconcile-apply.3.scdoc; do
  [ -s "$root/$required" ] || fail "missing or empty $required"
done
for document in README.md DESIGN.md HISTORY.md CONTRIBUTING.md MAINTAINING.md TESTING.md; do
  case $(sed -n '1p' "$root/$document") in '# '*) ;; *) fail "$document does not start with an ATX level-one heading" ;; esac
done

provider_target='libpkgreconcile-apply/managed-target/v1'
provider_object='libpkgreconcile-apply/rejected-object/v1'
for document in "$root/README.md" "$root/DESIGN.md" "$root/man/libpkgreconcile-apply.3.scdoc"; do
  grep -F "$provider_target" "$document" >/dev/null || fail "$(basename "$document") omits managed-target provider"
  grep -F "$provider_object" "$document" >/dev/null || fail "$(basename "$document") omits rejected-object provider"
done

grep -F '32-byte' "$root/README.md" >/dev/null || fail 'README omits target digest width'
grep -F '64 bytes' "$root/README.md" >/dev/null || fail 'README omits object locator width'
grep -F '32-byte' "$root/man/libpkgreconcile-apply.3.scdoc" >/dev/null || fail 'manual omits target digest width'
grep -F '64 bytes' "$root/man/libpkgreconcile-apply.3.scdoc" >/dev/null || fail 'manual omits object locator width'
grep -F 'durable protocol values' "$root/README.md" >/dev/null || fail 'README does not identify durable reference protocol'
grep -F 'Durable reference protocol' "$root/DESIGN.md" >/dev/null || fail 'DESIGN omits durable reference protocol ownership'
grep -F '## Protocol' "$root/TESTING.md" >/dev/null || fail 'TESTING omits protocol suite'

for document in "$root/README.md" "$root/DESIGN.md" "$root/man/libpkgreconcile-apply.3.scdoc"; do
  grep -Ei 'operation[- ]plan' "$document" >/dev/null || fail "$(basename "$document") omits retained plan authority"
  grep -Ei 'application[- ]attempt' "$document" >/dev/null || fail "$(basename "$document") omits retained attempt authority"
done

# The adapter owns projection, not mechanisms or durable state. Public docs must
# state those negative boundaries explicitly rather than implying them by omission.
grep -F 'does not open a rejected-object store' "$root/README.md" >/dev/null || fail 'README omits rejected-store opening boundary'
grep -F 'persist reconciliation state' "$root/README.md" >/dev/null || fail 'README omits persistence boundary'
grep -F 'reopen rejected bytes' "$root/DESIGN.md" >/dev/null || fail 'DESIGN omits reopening boundary'
grep -F 'persist or merge reconciliation inventories' "$root/DESIGN.md" >/dev/null || fail 'DESIGN omits persistence boundary'
grep -F 'does not open rejected objects' "$root/man/libpkgreconcile-apply.3.scdoc" >/dev/null || fail 'manual omits rejected-object opening boundary'
grep -F 'persist inventories' "$root/man/libpkgreconcile-apply.3.scdoc" >/dev/null || fail 'manual omits persistence boundary'

if grep -RInE '/var/lib/pkg/rejected|rejected-v[0-9]|generations/|renameat2|flock\(' \
    "$root/README.md" "$root/DESIGN.md" "$root/TESTING.md" "$root/MAINTAINING.md" "$root/man" >/dev/null; then
  fail 'provider-private storage grammar leaked into public documentation'
fi

printf '%s\n' 'documentation-source-contract: ok'
