# Testing

The test tree is divided by responsibility.

## Unit

`unit/` qualifies the adapter-owned binary reference schemas and strict inverse
translation. Foreign provider identifiers and malformed payload lengths are
refused.

## Integration

`integration/` uses real planner and application value objects. It proves:

- a planner-produced protected incoming upgrade projects to `incoming`;
- a planner-produced preserved old removal projects to `prior_installed`;
- exact managed-target identity survives projection;
- exact rejected-store and record identities survive locator round-trip; and
- completed evidence bound to another target context is refused.

These are composition tests. They do not require a POSIX rejected store.

## Header

Each installed public header is independently consumable.

## Contract

Contracts police architecture, exact ELF ABI, pkg-config closure, release
metadata, repository shape, style, documentation truth, CI qualification, and
test topology.

There is intentionally no mechanism or protocol suite. This repository owns no
filesystem mechanism and no durable serialization protocol.
