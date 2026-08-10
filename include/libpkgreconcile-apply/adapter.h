// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

/** @file adapter.h
 *  @brief Projection of completed application rejection into reconciliation values.
 */
#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <libpkgapply/result.h>
#include <libpkgapply/target_context.h>
#include <libpkgreconcile/pending.h>
#include <libpkgreconcile-apply/export.h>

namespace pkgreconcile::apply_adapter {

/** Provider/schema for managed-target reconciliation references. */
inline constexpr std::string_view managed_target_reference_provider =
    "libpkgreconcile-apply/managed-target/v1";

/** Provider/schema for rejected-store plus rejected-record locators. */
inline constexpr std::string_view rejected_object_locator_provider =
    "libpkgreconcile-apply/rejected-object/v1";

/** Stable reason that application-to-reconciliation projection was refused. */
enum class projection_error_code : std::uint8_t {
  target_binding_mismatch = 1,     ///< Evidence names another target context.
  unsupported_rejected_outcome = 2, ///< Rejected evidence has no retained side.
  target_reference_invalid = 3,    ///< Target provider or payload is invalid.
  object_locator_invalid = 4,      ///< Object provider or payload is invalid.
  pending_construction = 5,        ///< Core pending value refused projected data.
};

/** Typed application-to-reconciliation projection failure. */
class PKGRECONCILE_APPLY_API projection_error final : public std::invalid_argument {
public:
  /**
   * Construct one typed projection refusal.
   * @param code Stable refusal category.
   * @param message Human-readable diagnostic.
   */
  projection_error(projection_error_code code, std::string message);
  /** Destroy the polymorphic refusal. */
  ~projection_error() override;
  /** @return Stable refusal category. */
  [[nodiscard]] projection_error_code code() const noexcept;
private:
  projection_error_code code_;
};

/** Exact application authority encoded by one rejected-object locator. */
class PKGRECONCILE_APPLY_API rejected_object_reference final {
public:
  /** @return Exact rejected-object store identity. */
  [[nodiscard]] const pkgapply::rejected_object_store_identity& store() const noexcept;
  /** @return Exact immutable rejected-object record identity. */
  [[nodiscard]] const pkgapply::rejected_object_record_identity& record() const noexcept;
private:
  friend PKGRECONCILE_APPLY_API rejected_object_reference
  decode_rejected_object_locator(const rejected_object_locator& locator);
  rejected_object_reference(pkgapply::rejected_object_store_identity store,
                            pkgapply::rejected_object_record_identity record);
  pkgapply::rejected_object_store_identity store_;
  pkgapply::rejected_object_record_identity record_;
};

/** Target-bound pending values projected from one completed application. */
class PKGRECONCILE_APPLY_API completed_reconciliation_projection final {
public:
  /** @return Exact managed-target reconciliation reference. */
  [[nodiscard]] const reconciliation_target_reference& target() const noexcept;
  /** @return Exact operation-plan identity retained by completed evidence. */
  [[nodiscard]] const pkgplan::operation_plan_identity& plan() const noexcept;
  /** @return Exact physical application-attempt identity. */
  [[nodiscard]] const pkgapply::application_attempt_identity&
  attempt() const noexcept;
  /** @return Pending values in completed-evidence path order. */
  [[nodiscard]] const std::vector<pending_reconciliation>& pending() const noexcept;
private:
  friend PKGRECONCILE_APPLY_API completed_reconciliation_projection
  project_completed_application(const pkgapply::application_target_context& target,
                                const pkgapply::completed_application_evidence& evidence);
  completed_reconciliation_projection(
      reconciliation_target_reference target,
      pkgplan::operation_plan_identity plan,
      pkgapply::application_attempt_identity attempt,
      std::vector<pending_reconciliation> pending);
  reconciliation_target_reference target_;
  pkgplan::operation_plan_identity plan_;
  pkgapply::application_attempt_identity attempt_;
  std::vector<pending_reconciliation> pending_;
};

/**
 * Project one application managed-target identity into reconciliation authority.
 * @param target Exact managed target identity.
 * @return Adapter-owned target reference.
 * @throws projection_error if the reference cannot be represented.
 */
[[nodiscard]] PKGRECONCILE_APPLY_API reconciliation_target_reference
project_managed_target(const pkgapply::managed_target_identity& target);

/**
 * Decode this adapter's target reference back to exact application identity.
 * @param target Adapter-owned target reference.
 * @return Exact managed target identity.
 * @throws projection_error for another provider or malformed payload.
 */
[[nodiscard]] PKGRECONCILE_APPLY_API pkgapply::managed_target_identity
decode_managed_target(const reconciliation_target_reference& target);

/**
 * Encode exact rejected-store and record authority as one opaque locator.
 * @param store Exact rejected-object store identity.
 * @param record Exact immutable rejected-object record identity.
 * @return Adapter-owned opaque locator.
 * @throws projection_error if the locator cannot be represented.
 */
[[nodiscard]] PKGRECONCILE_APPLY_API rejected_object_locator
project_rejected_object(const pkgapply::rejected_object_store_identity& store,
                        const pkgapply::rejected_object_record_identity& record);

/**
 * Decode this adapter's rejected-object locator into exact application authority.
 * @param locator Adapter-owned opaque locator.
 * @return Exact store plus record reference.
 * @throws projection_error for another provider or malformed payload.
 */
[[nodiscard]] PKGRECONCILE_APPLY_API rejected_object_reference
decode_rejected_object_locator(const rejected_object_locator& locator);

/**
 * Project every completed rejected consequence into package-independent pending
 * reconciliation state.
 *
 * The target context must be the exact context named by @p evidence. Only
 * consequences with a completed rejected-object publication are projected.
 * The retained side comes from the accepted rejected outcome, never from path
 * role or package ownership inference.
 *
 * @param target Exact application target context used for store routing.
 * @param evidence Completed application evidence bound to @p target.
 * @return Target reference plus projected package-independent pending values.
 * @throws projection_error when target binding or rejected evidence is invalid.
 */
[[nodiscard]] PKGRECONCILE_APPLY_API completed_reconciliation_projection
project_completed_application(const pkgapply::application_target_context& target,
                              const pkgapply::completed_application_evidence& evidence);

} // namespace pkgreconcile::apply_adapter
