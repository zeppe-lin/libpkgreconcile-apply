// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include <libpkgreconcile-apply/adapter.h>

#include <array>
#include <filesystem>
#include <utility>

namespace pkgreconcile::apply_adapter {
namespace {
constexpr std::size_t digest_size = pkgapply::sha256_digest_size;
constexpr std::size_t rejected_locator_size = digest_size * 2U;

std::string digest_text(const std::uint8_t* bytes)
{
  static constexpr char hex[] = "0123456789abcdef";
  std::string value = "v1:sha256:";
  value.reserve(10U + digest_size * 2U);
  for (std::size_t i = 0; i < digest_size; ++i) {
    value.push_back(hex[(bytes[i] >> 4U) & 0x0fU]);
    value.push_back(hex[bytes[i] & 0x0fU]);
  }
  return value;
}

std::vector<std::uint8_t> bytes_of(const pkgapply::managed_target_identity& value)
{
  return {value.bytes().begin(), value.bytes().end()};
}

projection_error invalid(projection_error_code code, const char* message)
{
  return projection_error(code, message);
}

rejected_object_side side_for(pkgplan::planned_rejected_outcome outcome)
{
  switch (outcome) {
    case pkgplan::planned_rejected_outcome::stage_incoming:
      return rejected_object_side::incoming;
    case pkgplan::planned_rejected_outcome::stage_old:
      return rejected_object_side::prior_installed;
    case pkgplan::planned_rejected_outcome::none:
      break;
  }
  throw invalid(projection_error_code::unsupported_rejected_outcome,
                "completed rejected consequence has no retained side");
}
} // namespace

projection_error::projection_error(projection_error_code code, std::string message)
    : std::invalid_argument(std::move(message)), code_(code)
{
}
projection_error::~projection_error() = default;
projection_error_code projection_error::code() const noexcept { return code_; }

rejected_object_reference::rejected_object_reference(
    pkgapply::rejected_object_store_identity store,
    pkgapply::rejected_object_record_identity record)
    : store_(std::move(store)), record_(std::move(record))
{
}
const pkgapply::rejected_object_store_identity& rejected_object_reference::store() const noexcept { return store_; }
const pkgapply::rejected_object_record_identity& rejected_object_reference::record() const noexcept { return record_; }

completed_reconciliation_projection::completed_reconciliation_projection(
    reconciliation_target_reference target,
    pkgplan::operation_plan_identity plan,
    pkgapply::application_attempt_identity attempt,
    std::vector<pending_reconciliation> pending)
    : target_(std::move(target)), plan_(std::move(plan)),
      attempt_(std::move(attempt)), pending_(std::move(pending))
{
}
const reconciliation_target_reference& completed_reconciliation_projection::target() const noexcept { return target_; }
const pkgplan::operation_plan_identity&
completed_reconciliation_projection::plan() const noexcept
{
  return plan_;
}
const pkgapply::application_attempt_identity&
completed_reconciliation_projection::attempt() const noexcept
{
  return attempt_;
}
const std::vector<pending_reconciliation>&
completed_reconciliation_projection::pending() const noexcept
{
  return pending_;
}

reconciliation_target_reference
project_managed_target(const pkgapply::managed_target_identity& target)
{
  try {
    return reconciliation_target_reference::make(
        std::string(managed_target_reference_provider), bytes_of(target));
  } catch (const std::invalid_argument&) {
    throw invalid(projection_error_code::target_reference_invalid,
                  "managed target could not be represented for reconciliation");
  }
}

pkgapply::managed_target_identity
decode_managed_target(const reconciliation_target_reference& target)
{
  if (target.provider() != managed_target_reference_provider ||
      target.bytes().size() != digest_size)
    throw invalid(projection_error_code::target_reference_invalid,
                  "reconciliation target is not a libpkgapply managed-target reference");
  try {
    return pkgapply::managed_target_identity::parse(digest_text(target.bytes().data()));
  } catch (const std::invalid_argument&) {
    throw invalid(projection_error_code::target_reference_invalid,
                  "reconciliation target contains an invalid managed-target identity");
  }
}

rejected_object_locator
project_rejected_object(const pkgapply::rejected_object_store_identity& store,
                        const pkgapply::rejected_object_record_identity& record)
{
  std::vector<std::uint8_t> bytes;
  bytes.reserve(rejected_locator_size);
  bytes.insert(bytes.end(), store.bytes().begin(), store.bytes().end());
  bytes.insert(bytes.end(), record.bytes().begin(), record.bytes().end());
  try {
    return rejected_object_locator::make(
        std::string(rejected_object_locator_provider), std::move(bytes));
  } catch (const std::invalid_argument&) {
    throw invalid(projection_error_code::object_locator_invalid,
                  "rejected object could not be represented for reconciliation");
  }
}

rejected_object_reference
decode_rejected_object_locator(const rejected_object_locator& locator)
{
  if (locator.provider() != rejected_object_locator_provider ||
      locator.bytes().size() != rejected_locator_size)
    throw invalid(projection_error_code::object_locator_invalid,
                  "reconciliation object is not a libpkgapply rejected-object reference");
  try {
    const auto* bytes = locator.bytes().data();
    return rejected_object_reference(
        pkgapply::rejected_object_store_identity::parse(digest_text(bytes)),
        pkgapply::rejected_object_record_identity::parse(digest_text(bytes + digest_size)));
  } catch (const std::invalid_argument&) {
    throw invalid(projection_error_code::object_locator_invalid,
                  "reconciliation object contains invalid application identities");
  }
}

completed_reconciliation_projection
project_completed_application(const pkgapply::application_target_context& target,
                              const pkgapply::completed_application_evidence& evidence)
{
  if (target.identity() != evidence.target())
    throw invalid(projection_error_code::target_binding_mismatch,
                  "completed application evidence names another target context");

  auto target_reference = project_managed_target(target.managed_target());
  std::vector<pending_reconciliation> pending;
  pending.reserve(evidence.paths().size());
  for (const auto& consequence : evidence.paths()) {
    if (!consequence.rejected_object())
      continue;
    try {
      pending.push_back(pending_reconciliation::make(
          target_reference,
          std::filesystem::path(consequence.path().string()),
          side_for(consequence.requested_rejected()),
          project_rejected_object(target.rejected_store(),
                                  *consequence.rejected_object())));
    } catch (const projection_error&) {
      throw;
    } catch (const std::invalid_argument&) {
      throw invalid(projection_error_code::pending_construction,
                    "completed rejected consequence could not form reconciliation state");
    }
  }
  return completed_reconciliation_projection(
      std::move(target_reference), evidence.plan(), evidence.attempt(),
      std::move(pending));
}

} // namespace pkgreconcile::apply_adapter
