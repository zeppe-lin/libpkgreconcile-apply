# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

if get_option('tests').allowed()
  sh = find_program('sh')

  unit_tests = [
    ['error-model', 'unit/error_model_test.cpp'],
  ]
  foreach item : unit_tests
    target = executable(
      item[0] + '-test',
      item[1],
      dependencies: libpkgreconcile_apply_dep,
      cpp_args: ['-UNDEBUG'],
      install: false,
    )
    test(item[0], target, suite: 'unit')
  endforeach

  protocol_test = executable(
    'reference-schema-test',
    'protocol/reference_schema_test.cpp',
    dependencies: libpkgreconcile_apply_dep,
    cpp_args: ['-UNDEBUG'],
    install: false,
  )
  test('reference-schema', protocol_test, suite: 'protocol')

  # This suite composes real planner/application semantics. These dependencies
  # are qualification-only and do not enter the product dependency boundary.
  libpkgsource_test_dep = dependency('libpkgsource', version: ['>=4.0.0', '<5.0.0'], required: true, static: want_static)
  libpkgcatalog_test_dep = dependency('libpkgcatalog', version: ['>=4.0.0', '<5.0.0'], required: true, static: want_static)
  libpkgresolve_test_dep = dependency('libpkgresolve', version: ['>=4.0.0', '<5.0.0'], required: true, static: want_static)
  libpkgbuild_test_dep = dependency('libpkgbuild', version: ['>=3.0.1', '<4.0.0'], required: true, static: want_static)
  libpkgimage_test_dep = dependency('libpkgimage', version: ['>=0.4.1', '<1.0.0'], required: true, static: want_static)
  libpkgplan_test_dep = dependency('libpkgplan', version: ['>=0.3.1', '<1.0.0'], required: true, static: want_static)
  libpkgbuild_image_test_dep = dependency('libpkgbuild-image', version: ['>=1.0.1', '<2.0.0'], required: true, static: want_static)
  libpkgsource_plan_test_dep = dependency('libpkgsource-plan', version: ['>=2.0.0', '<3.0.0'], required: true, static: want_static)
  libpkgbuild_plan_test_dep = dependency('libpkgbuild-plan', version: ['>=1.1.0', '<2.0.0'], required: true, static: want_static)

  integration_deps = [
    libpkgreconcile_apply_dep,
    libpkgsource_test_dep,
    libpkgcatalog_test_dep,
    libpkgresolve_test_dep,
    libpkgbuild_test_dep,
    libpkgimage_test_dep,
    libpkgplan_test_dep,
    libpkgbuild_image_test_dep,
    libpkgsource_plan_test_dep,
    libpkgbuild_plan_test_dep,
  ]
  integration_test = executable(
    'completed-projection-integration-test',
    'integration/completed_projection_test.cpp',
    dependencies: integration_deps,
    cpp_args: ['-UNDEBUG'],
    install: false,
  )
  test('completed-projection', integration_test, suite: 'integration')

  public_headers = [
    ['export', 'libpkgreconcile-apply/export.h'],
    ['adapter', 'libpkgreconcile-apply/adapter.h'],
    ['umbrella', 'libpkgreconcile-apply/libpkgreconcile-apply.h'],
  ]
  foreach item : public_headers
    target = executable(
      'header-' + item[0] + '-test',
      'header/public_header_test.cpp',
      dependencies: libpkgreconcile_apply_dep,
      cpp_args: ['-DLIBPKGRECONCILE_APPLY_TEST_HEADER="' + item[1] + '"'],
      install: false,
    )
    test('header-' + item[0], target, suite: 'header')
  endforeach

  if library_kind == 'shared'
    nm = find_program('nm')
    test(
      'abi-surface',
      sh,
      args: [
        meson.current_source_dir() / 'contracts/check_abi_surface.sh',
        nm.full_path(),
        pkg_reconcile_apply.full_path(),
        meson.project_source_root() / 'abi/libpkgreconcile-apply.exports',
      ],
      suite: 'contract',
    )
  endif

  foreach contract : [
    'architecture_contract',
    'ci_qualification_source',
    'documentation_source',
    'doxygen_contract',
    'release_metadata',
    'repository_contract',
    'style_contract',
    'test_layout',
  ]
    test(
      contract.replace('_', '-'),
      sh,
      args: [
        meson.current_source_dir() / ('contracts/check_' + contract + '.sh'),
        meson.project_source_root(),
      ],
      suite: 'contract',
    )
  endforeach

  test(
    'pkgconfig-metadata',
    sh,
    args: [
      meson.current_source_dir() / 'contracts/check_pkgconfig_metadata.sh',
      meson.project_build_root(),
      meson.project_version(),
    ],
    suite: 'contract',
  )
endif
