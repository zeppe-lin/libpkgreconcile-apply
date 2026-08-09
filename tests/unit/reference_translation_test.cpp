// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../support/test.hpp"

#include <libpkgreconcile-apply/adapter.h>

#include <cstdint>
#include <string>

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
}

int main()
{
  using namespace pkgreconcile::apply_adapter;
  test_support::runner runner;

  const auto managed = identity<pkgapply::managed_target_identity>(1);
  const auto target = project_managed_target(managed);
  runner.run("target provider is stable", [&] {
    TEST_CHECK(target.provider() == managed_target_reference_provider);
    TEST_CHECK(target.bytes().size() == pkgapply::sha256_digest_size);
  });
  runner.run("managed target round-trips exactly", [&] {
    TEST_CHECK(decode_managed_target(target) == managed);
  });

  const auto store = identity<pkgapply::rejected_object_store_identity>(40);
  const auto record = identity<pkgapply::rejected_object_record_identity>(80);
  const auto locator = project_rejected_object(store, record);
  runner.run("object provider is stable", [&] {
    TEST_CHECK(locator.provider() == rejected_object_locator_provider);
    TEST_CHECK(locator.bytes().size() == pkgapply::sha256_digest_size * 2U);
  });
  runner.run("store and record round-trip exactly", [&] {
    const auto decoded = decode_rejected_object_locator(locator);
    TEST_CHECK(decoded.store() == store);
    TEST_CHECK(decoded.record() == record);
  });
  runner.run("foreign target provider is refused", [&] {
    const auto wrong = pkgreconcile::reconciliation_target_reference::make(
        "other/provider/v1", target.bytes());
    TEST_CHECK_THROWS((void)decode_managed_target(wrong), projection_error);
  });
  runner.run("malformed target payload is refused", [&] {
    const auto wrong = pkgreconcile::reconciliation_target_reference::make(
        std::string(managed_target_reference_provider), {1, 2, 3});
    TEST_CHECK_THROWS((void)decode_managed_target(wrong), projection_error);
  });
  runner.run("foreign object provider is refused", [&] {
    const auto wrong = pkgreconcile::rejected_object_locator::make(
        "other/provider/v1", locator.bytes());
    TEST_CHECK_THROWS((void)decode_rejected_object_locator(wrong),
                      projection_error);
  });
  runner.run("malformed object payload is refused", [&] {
    const auto wrong = pkgreconcile::rejected_object_locator::make(
        std::string(rejected_object_locator_provider), {1, 2, 3});
    TEST_CHECK_THROWS((void)decode_rejected_object_locator(wrong),
                      projection_error);
  });

  return runner.finish();
}
