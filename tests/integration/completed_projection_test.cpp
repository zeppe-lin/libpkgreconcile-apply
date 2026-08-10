// SPDX-FileCopyrightText: 2026 Alexandr Savca
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../fixtures/plan.h"
#include "../support/test.hpp"

#include <libpkgreconcile-apply/adapter.h>
#include <libpkgapply/execution_control.h>
#include <libpkgapply/request.h>
#include <libpkgapply/result.h>

#include <array>
#include <cstdint>
#include <string>

namespace {
template<class Identity>
Identity app_identity(std::uint8_t seed)
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
Identity plan_identity(std::uint8_t seed)
{
  std::array<std::uint8_t, 32> bytes{};
  for (std::size_t i = 0; i < bytes.size(); ++i)
    bytes[i] = static_cast<std::uint8_t>(seed + i);
  return Identity::from_sha256(bytes);
}

pkgapply::application_target_context target(std::uint8_t seed = 1)
{
  return pkgapply::application_target_context::make(
      plan_identity<pkgplan::target_system_context_identity>(seed),
      app_identity<pkgapply::managed_target_identity>(seed + 1),
      app_identity<pkgapply::root_view_identity>(seed + 2),
      app_identity<pkgapply::observation_backend_identity>(seed + 3),
      app_identity<pkgapply::mutation_backend_identity>(seed + 4),
      app_identity<pkgapply::mutation_exclusion_domain_identity>(seed + 5),
      app_identity<pkgapply::active_object_namespace_identity>(seed + 6),
      app_identity<pkgapply::rejected_object_store_identity>(seed + 7),
      app_identity<pkgapply::staging_namespace_identity>(seed + 8),
      app_identity<pkgapply::journal_namespace_identity>(seed + 9),
      app_identity<pkgapply::execution_capability_profile_identity>(seed + 10));
}

pkgapply::application_execution_control control()
{
  return pkgapply::application_execution_control::make(
      pkgapply::application_recovery_requirement::best_effort,
      pkgapply::application_durability_requirement::all_application_domains,
      pkgapply::application_cancellation_policy::recover_after_target_mutation);
}

pkgapply::application_durability_profile durability()
{
  using domain = pkgapply::application_durability_domain;
  using status = pkgapply::application_durability_status;
  return pkgapply::application_durability_profile({
      {domain::journal, status::confirmed},
      {domain::incoming_staging, status::confirmed},
      {domain::recovery_staging, status::confirmed},
      {domain::active_namespace, status::confirmed},
      {domain::rejected_object_store, status::confirmed},
      {domain::completed_evidence, status::confirmed},
  });
}

pkgapply::completed_object_fact
regular(const pkgplan::package_path& path, std::uint8_t content)
{
  return pkgapply::completed_object_fact(
      path,
      pkgapply::completed_object_kind::regular,
      pkgapply::qualified_fact<std::uint32_t>::known(0644),
      pkgapply::qualified_fact<std::uint64_t>::known(0),
      pkgapply::qualified_fact<std::uint64_t>::known(0),
      pkgapply::qualified_fact<std::uint64_t>::known(4),
      pkgapply::qualified_fact<pkgapply::completed_object_timestamp>::known(
          {10, 0}),
      pkgapply::qualified_fact<pkgapply::completed_regular_content_identity>::
          known(app_identity<pkgapply::completed_regular_content_identity>(
              content)),
      pkgapply::qualified_fact<std::string>::not_applicable(),
      pkgapply::qualified_fact<pkgapply::completed_device_number>::
          not_applicable(),
      pkgapply::qualified_fact<pkgapply::completed_hardlink_relation>::unknown(),
      pkgapply::object_fact_provenance::application_observation,
      pkgapply::object_fact_completeness::complete);
}
}

int main()
{
  using namespace pkgapply::test::fixture;
  using namespace pkgreconcile::apply_adapter;
  test_support::runner runner;

  auto context = target();
  const auto path = pkgplan::package_path::parse("tool");
  planning_authorities authorities(context.target());
  auto policy = policy_snapshot(
      authorities,
      path_policy(pkgplan::incoming_path_policy::retain(
          pkgplan::rejected_object_policy::stage,
          pkgplan::retained_active_ownership_policy::
              do_not_claim_operated_package)));
  const auto active = regular_object(1, 0755);
  const auto plan = upgrade_plan(
      authorities,
      {regular_entry(path.string(), 2, 0755)},
      {pkgplan::target_path_observation::present(
          pkgplan::filesystem_object_fact(path, active))},
      {pkgplan::installed_ownership_claim(
          path, authorities.installed_package, active)},
      std::move(policy));
  const auto& decision = plan.paths().front();
  runner.run("planner selected incoming rejection", [&] {
    TEST_CHECK(decision.rejected() ==
               pkgplan::planned_rejected_outcome::stage_incoming);
  });

  auto request = pkgapply::upgrade_application_request::make(
      plan,
      incoming_package({regular_entry(path.string(), 2, 0755)},
                       archive_digest(),
                       "2.0"),
      context,
      control());
  const auto record =
      app_identity<pkgapply::rejected_object_record_identity>(90);
  const auto before = pkgapply::application_path_observation::present(
      regular(path, 1));
  const auto consequence = pkgapply::application_path_consequence(
      path,
      pkgapply::application_path_role::incoming_entry,
      decision.active(),
      decision.rejected(),
      decision.incoming_entry(),
      decision.ownership(),
      pkgapply::application_effect_status::completed,
      pkgapply::application_effect_status::completed,
      before,
      before,
      record,
      pkgapply::ownership_publication_status::eligible);
  const auto evidence = pkgapply::completed_application_evidence::upgrade(
      request,
      app_identity<pkgapply::application_attempt_identity>(60),
      app_identity<pkgapply::lease_bound_state_projection_identity>(61),
      app_identity<pkgapply::application_journal_identity>(62),
      {consequence},
      durability());

  const auto projection = project_completed_application(context, evidence);
  runner.run("projection retains exact managed target", [&] {
    TEST_CHECK(decode_managed_target(projection.target()) ==
               context.managed_target());
  });
  runner.run("projection retains exact plan and attempt", [&] {
    TEST_CHECK(projection.plan() == evidence.plan());
    TEST_CHECK(projection.attempt() == evidence.attempt());
  });
  runner.run("completed rejection becomes one pending value", [&] {
    TEST_CHECK(projection.pending().size() == 1U);
  });
  const auto& pending = projection.pending().front();
  runner.run("pending path is canonical application path", [&] {
    TEST_CHECK(pending.path().native() == path.string());
  });
  runner.run("incoming rejection maps to incoming side", [&] {
    TEST_CHECK(pending.side() ==
               pkgreconcile::rejected_object_side::incoming);
  });
  runner.run("pending locator retains exact store and record", [&] {
    const auto object = decode_rejected_object_locator(pending.object());
    TEST_CHECK(object.store() == context.rejected_store());
    TEST_CHECK(object.record() == record);
  });
  runner.run("another target context is refused", [&] {
    auto other = target(40);
    try {
      (void)project_completed_application(other, evidence);
      TEST_CHECK(false);
    } catch (const projection_error& error) {
      TEST_CHECK(error.code() ==
                 projection_error_code::target_binding_mismatch);
    }
  });

  const auto old_policy = policy_snapshot(
      authorities,
      path_policy(pkgplan::incoming_path_policy::activate(),
                  pkgplan::obsolete_path_policy::remove(
                      pkgplan::rejected_object_policy::stage)));
  const auto removal = removal_plan(
      authorities,
      {pkgplan::installed_ownership_claim(
          path, authorities.installed_package, active)},
      {pkgplan::target_path_observation::present(
          pkgplan::filesystem_object_fact(path, active))},
      std::move(old_policy));
  const auto& removal_decision = removal.paths().front();
  runner.run("planner selected old-object rejection", [&] {
    TEST_CHECK(removal_decision.rejected() ==
               pkgplan::planned_rejected_outcome::stage_old);
  });

  const auto removal_request = pkgapply::removal_application_request::make(
      removal, context, control());
  const auto old_record =
      app_identity<pkgapply::rejected_object_record_identity>(100);
  const auto old_consequence = pkgapply::application_path_consequence(
      path,
      pkgapply::application_path_role::installed_owned_path,
      removal_decision.active(),
      removal_decision.rejected(),
      std::nullopt,
      removal_decision.ownership(),
      pkgapply::application_effect_status::completed,
      pkgapply::application_effect_status::completed,
      before,
      pkgapply::application_path_observation::absent(path),
      old_record,
      pkgapply::ownership_publication_status::eligible);
  const auto removal_evidence =
      pkgapply::completed_application_evidence::removal(
          removal_request,
          app_identity<pkgapply::application_attempt_identity>(70),
          app_identity<pkgapply::lease_bound_state_projection_identity>(71),
          app_identity<pkgapply::application_journal_identity>(72),
          {old_consequence},
          durability());
  const auto old_projection =
      project_completed_application(context, removal_evidence);
  runner.run("old rejection maps to prior-installed side", [&] {
    TEST_CHECK(old_projection.pending().size() == 1U);
    TEST_CHECK(old_projection.pending().front().side() ==
               pkgreconcile::rejected_object_side::prior_installed);
  });
  runner.run("old locator retains exact store and record", [&] {
    const auto object = decode_rejected_object_locator(
        old_projection.pending().front().object());
    TEST_CHECK(object.store() == context.rejected_store());
    TEST_CHECK(object.record() == old_record);
  });

  return runner.finish();
}
