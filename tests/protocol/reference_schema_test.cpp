// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../support/test.hpp"

#include <libpkgreconcile-apply/adapter.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace {
template<class Identity>
Identity identity(std::uint8_t seed)
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

template<class Identity>
std::vector<std::uint8_t> bytes_of(const Identity& value)
{
  return {value.bytes().begin(), value.bytes().end()};
}
}

int main()
{
  using namespace pkgreconcile::apply_adapter;
  test_support::runner runner;

  const auto managed = identity<pkgapply::managed_target_identity>(1);
  const auto managed_bytes = bytes_of(managed);
  const auto target = project_managed_target(managed);

  runner.run("managed-target protocol is exact", [&] {
    TEST_CHECK(target.provider() == managed_target_reference_provider);
    TEST_CHECK(target.bytes() == managed_bytes);
  });
  runner.run("canonical managed-target payload decodes independently", [&] {
    const auto encoded = pkgreconcile::reconciliation_target_reference::make(
        std::string(managed_target_reference_provider), managed_bytes);
    TEST_CHECK(decode_managed_target(encoded) == managed);
  });

  const auto store = identity<pkgapply::rejected_object_store_identity>(40);
  const auto record = identity<pkgapply::rejected_object_record_identity>(80);
  std::vector<std::uint8_t> locator_bytes = bytes_of(store);
  const auto record_bytes = bytes_of(record);
  locator_bytes.insert(locator_bytes.end(), record_bytes.begin(), record_bytes.end());
  const auto locator = project_rejected_object(store, record);

  runner.run("rejected-object protocol is store then record", [&] {
    TEST_CHECK(locator.provider() == rejected_object_locator_provider);
    TEST_CHECK(locator.bytes() == locator_bytes);
  });
  runner.run("canonical rejected-object payload decodes independently", [&] {
    const auto encoded = pkgreconcile::rejected_object_locator::make(
        std::string(rejected_object_locator_provider), locator_bytes);
    const auto decoded = decode_rejected_object_locator(encoded);
    TEST_CHECK(decoded.store() == store);
    TEST_CHECK(decoded.record() == record);
  });

  runner.run("foreign target provider is refused", [&] {
    const auto wrong = pkgreconcile::reconciliation_target_reference::make(
        "other/provider/v1", managed_bytes);
    try {
      (void)decode_managed_target(wrong);
      TEST_CHECK(false);
    } catch (const projection_error& error) {
      TEST_CHECK(error.code() == projection_error_code::target_reference_invalid);
    }
  });
  runner.run("malformed target payload is refused", [&] {
    const auto wrong = pkgreconcile::reconciliation_target_reference::make(
        std::string(managed_target_reference_provider), {1, 2, 3});
    try {
      (void)decode_managed_target(wrong);
      TEST_CHECK(false);
    } catch (const projection_error& error) {
      TEST_CHECK(error.code() == projection_error_code::target_reference_invalid);
    }
  });
  runner.run("foreign object provider is refused", [&] {
    const auto wrong = pkgreconcile::rejected_object_locator::make(
        "other/provider/v1", locator_bytes);
    try {
      (void)decode_rejected_object_locator(wrong);
      TEST_CHECK(false);
    } catch (const projection_error& error) {
      TEST_CHECK(error.code() == projection_error_code::object_locator_invalid);
    }
  });
  runner.run("malformed object payload is refused", [&] {
    const auto wrong = pkgreconcile::rejected_object_locator::make(
        std::string(rejected_object_locator_provider), {1, 2, 3});
    try {
      (void)decode_rejected_object_locator(wrong);
      TEST_CHECK(false);
    } catch (const projection_error& error) {
      TEST_CHECK(error.code() == projection_error_code::object_locator_invalid);
    }
  });

  return runner.finish();
}
