# Design

## Authority flow

```text
completed_application_evidence
        +
application_target_context
        |
        v
libpkgreconcile-apply
        |
        +-- managed target reference
        +-- rejected store + record locator
        +-- canonical path
        +-- retained side
        v
pending_reconciliation[]
```

The adapter is a composition boundary, not an actuator and not a persistence
provider.

## Target identity

The complete `application_target_context` is intentionally not used as the
reconciliation target identity. Observation backend, mutation backend, staging
namespace, journal namespace, or lifecycle executor may change without making
the managed filesystem target a different reconciliation target.

The exact `managed_target_identity` is therefore the target authority projected
into `libpkgreconcile`. The adapter-owned provider/schema identifier is
`libpkgreconcile-apply/managed-target/v1`.

## Rejected-object identity

A rejected record identity alone is scoped by a rejected-object store chosen by
orchestration. The reconciliation locator therefore retains both
`rejected_object_store_identity` and `rejected_object_record_identity`.

The encoding is adapter-owned and fixed-width under provider/schema
`libpkgreconcile-apply/rejected-object/v1`. Consumers must use the decoder;
they must not infer a filesystem pathname or enumerate store contents.

## Evidence admission

`project_completed_application()` requires the supplied target context identity
to equal the target context identity retained by completed evidence. It then
projects only path consequences that carry a completed rejected-object record.

The completed-evidence constructor in `libpkgapply` is the authority that proves
those path consequences are complete and internally coherent. This adapter does
not re-plan the operation or reinterpret policy reasons.

## Non-goals

This library does not:

- reopen rejected bytes;
- validate a live rejected store;
- persist or merge reconciliation inventories;
- suppress replay after resolution;
- inspect package state;
- invent package ownership;
- compare rejected and active bytes;
- expose a CLI; or
- coordinate transactions.
