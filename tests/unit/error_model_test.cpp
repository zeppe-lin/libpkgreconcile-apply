// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../support/test.hpp"
#include <libpkgreconcile-apply/adapter.h>

#include <cstdint>
#include <string>

namespace {
template<class Identity> Identity identity(std::uint8_t seed)
{
  std::string text = "v1:sha256:";
  constexpr char hex[] = "0123456789abcdef";
  for (std::size_t i = 0; i < 32; ++i) {
    const auto byte = static_cast<std::uint8_t>(seed + i);
    text.push_back(hex[(byte >> 4U) & 0x0fU]);
    text.push_back(hex[byte & 0x0fU]);
  }
  return Identity::parse(text);
}
}

int main()
{
  using namespace pkgreconcile::apply_adapter;
  test_support::runner runner;

  runner.run("projection error retains stable code", [&] {
    projection_error error(projection_error_code::object_locator_invalid,
                           "fixture refusal");
    TEST_CHECK(error.code() == projection_error_code::object_locator_invalid);
    TEST_CHECK(std::string(error.what()) == "fixture refusal");
  });

  runner.run("foreign target refusal is typed", [&] {
    const auto managed = identity<pkgapply::managed_target_identity>(1);
    const auto projected = project_managed_target(managed);
    const auto foreign = pkgreconcile::reconciliation_target_reference::make(
        "fixture/foreign-target/v1", projected.bytes());
    try {
      (void)decode_managed_target(foreign);
      TEST_CHECK(false);
    } catch (const projection_error& error) {
      TEST_CHECK(error.code() == projection_error_code::target_reference_invalid);
    }
  });

  runner.run("foreign object refusal is typed", [&] {
    const auto store = identity<pkgapply::rejected_object_store_identity>(40);
    const auto record = identity<pkgapply::rejected_object_record_identity>(80);
    const auto projected = project_rejected_object(store, record);
    const auto foreign = pkgreconcile::rejected_object_locator::make(
        "fixture/foreign-object/v1", projected.bytes());
    try {
      (void)decode_rejected_object_locator(foreign);
      TEST_CHECK(false);
    } catch (const projection_error& error) {
      TEST_CHECK(error.code() == projection_error_code::object_locator_invalid);
    }
  });

  return runner.finish();
}
