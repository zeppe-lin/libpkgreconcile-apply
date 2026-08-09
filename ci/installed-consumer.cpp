// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgreconcile-apply/libpkgreconcile-apply.h>

#include <string>

int main()
{
  const auto identity = pkgapply::managed_target_identity::parse(
      "v1:sha256:000102030405060708090a0b0c0d0e0f"
      "101112131415161718191a1b1c1d1e1f");
  const auto reference = pkgreconcile::apply_adapter::project_managed_target(identity);
  return pkgreconcile::apply_adapter::decode_managed_target(reference) == identity ? 0 : 1;
}
