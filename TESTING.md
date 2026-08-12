# Testing

The test tree is divided by responsibility.

## Unit

`unit/` qualifies the stable typed projection error model.

## Protocol

`protocol/` qualifies the adapter-owned durable opaque reference schemas. The
managed-target payload is exactly the 32 digest bytes. The rejected-object
payload is exactly the 32 rejected-store digest bytes followed by the 32
rejected-record digest bytes. Decoder cases are built independently of the
encoders so an encoder/decoder pair cannot drift together unnoticed. Foreign
providers and malformed payload lengths are refused.

## Integration

`integration/` uses real planner and application value objects. It proves:

- a planner-produced protected incoming upgrade projects to `incoming`;
- a planner-produced preserved old removal projects to `prior_installed`;
- exact managed-target identity survives projection;
- exact plan and physical attempt identities survive projection;
- exact rejected-store and record identities survive locator round-trip; and
- completed evidence bound to another target context is refused.

These are composition tests. They do not require a POSIX rejected store.

## Header

Each installed public header is independently consumable.

## Contract

Contracts police architecture, exact ELF ABI, pkg-config closure, release
metadata, repository shape, style, documentation truth, CI qualification, and
test topology.

There is intentionally no mechanism suite. This repository owns no filesystem
mechanism and no standalone durable framing codec, but its opaque reference
payloads are durable protocol values because reconciliation providers persist
them verbatim.
