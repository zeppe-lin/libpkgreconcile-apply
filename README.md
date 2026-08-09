# libpkgreconcile-apply

`libpkgreconcile-apply` is the native C++17 composition boundary from completed
`libpkgapply` application evidence to package-independent `libpkgreconcile`
pending values.

It translates only authority already present in a completed application:

- the orchestrator-managed target identity;
- the rejected-object store identity;
- the immutable rejected-object record identity;
- the canonical operated package path; and
- the accepted rejected side (`stage_incoming` or `stage_old`).

The library does not open a rejected-object store, parse a provider-private
filesystem layout, persist reconciliation state, inspect installed package
state, infer package ownership, or choose a user disposition.

## Projection

A completed rejected consequence becomes one `pending_reconciliation`.

The target reference uses provider
`libpkgreconcile-apply/managed-target/v1` and contains the exact 32-byte
`pkgapply::managed_target_identity` digest.

The rejected-object locator uses provider
`libpkgreconcile-apply/rejected-object/v1` and contains exactly 64 bytes:

1. the 32-byte `pkgapply::rejected_object_store_identity` digest;
2. the 32-byte `pkgapply::rejected_object_record_identity` digest.

The locator therefore identifies both the external rejected store and the
immutable record inside that store. It does not encode a pathname or store
implementation.

`decode_managed_target()` and `decode_rejected_object_locator()` are the
canonical inverse operations for these adapter-owned schemas.

## Side mapping

The retained side is derived only from the accepted rejected outcome:

- `stage_incoming` -> `rejected_object_side::incoming`;
- `stage_old` -> `rejected_object_side::prior_installed`.

Path roles and ownership transitions are not used as provenance guesses.

## Boundary

`libpkgreconcile-apply` depends on `libpkgapply` and `libpkgreconcile` only.
It deliberately does not depend on `libpkgapply-posix`, `libpkgreconcile-posix`,
`libpkgstate`, or `pkgctl`.

A POSIX composition layer may decode the locator, resolve its store identity to
an exact `libpkgapply-posix` rejected-store authority, and then use the record
identity with that provider's direct identity reopening API. That routing is
outside this library.

## Building

```sh
meson setup build
meson compile -C build
meson test -C build --print-errorlogs
```

Static builds use matching `default_library=static` and `link_mode=static`.
