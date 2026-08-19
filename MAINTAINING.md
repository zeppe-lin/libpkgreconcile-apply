# Maintaining

Preserve the composition boundary.

A change belongs here only when it translates already-established
`libpkgapply` authority into `libpkgreconcile` values or decodes this adapter's
own opaque reference schemas.

Do not add filesystem discovery, rejected-store path knowledge, state-database
access, transaction orchestration, or user-interface decisions.

Changing either provider identifier or the byte layout behind it is a protocol
change even though `libpkgreconcile` treats the bytes as opaque. Before the first
production use, replace the current schema in place and keep it at version 1;
after production use, such a change requires explicit compatibility analysis.

The shared ABI is reviewed through `abi/libpkgreconcile-apply.exports`. Public
values retain application identities by value, so an application ABI-generation
change requires an explicit carrier-layout review before widening the accepted
dependency interval. The application-4 rebind preserves those layouts and keeps
provider SONAME 0. Shared qualification must bind the product to
`libpkgapply.so.4` and refuse older application generations.
