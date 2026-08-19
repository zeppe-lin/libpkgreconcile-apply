// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include <libpkgreconcile-apply/adapter.h>

#include <cstddef>

namespace {
template<class T, std::size_t Size, std::size_t Align>
constexpr void require_layout()
{
  static_assert(sizeof(T) == Size, "ABI size changed");
  static_assert(alignof(T) == Align, "ABI alignment changed");
}
}

int main()
{
#if defined(__x86_64__) || defined(_M_X64)
  require_layout<pkgapply::application_attempt_identity, 64, 8>();
  require_layout<pkgapply::rejected_object_store_identity, 64, 8>();
  require_layout<pkgapply::rejected_object_record_identity, 64, 8>();
  require_layout<pkgreconcile::apply_adapter::projection_error, 24, 8>();
  require_layout<pkgreconcile::apply_adapter::rejected_object_reference, 128, 8>();
  require_layout<
      pkgreconcile::apply_adapter::completed_reconciliation_projection, 176, 8>();
#endif
  return 0;
}
