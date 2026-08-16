# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

inc = include_directories('../include')

elf_export_generator = find_program(
  meson.project_source_root() / 'scripts/generate-elf-export-script.sh',
)
elf_export_script_name = 'libpkgreconcile-apply.exports.map'
elf_export_script_path = meson.current_build_dir() / elf_export_script_name
configure_file(
  input: meson.project_source_root() / 'abi/libpkgreconcile-apply.exports',
  output: elf_export_script_name,
  command: [elf_export_generator, '@INPUT@'],
  capture: true,
)

link_args = []
if library_kind == 'shared'
  link_args += ['-Wl,--version-script=' + elf_export_script_path]
endif

pkg_reconcile_apply = library(
  'pkgreconcile-apply',
  'adapter.cpp',
  include_directories: inc,
  dependencies: [libpkgreconcile_dep, libpkgapply_dep],
  cpp_args: ['-DLIBPKGRECONCILE_APPLY_BUILD'] + visibility_cpp_args,
  gnu_symbol_visibility: 'hidden',
  link_args: link_args,
  soversion: '0',
  version: meson.project_version(),
  install: true,
)

libpkgreconcile_apply_dep = declare_dependency(
  link_with: pkg_reconcile_apply,
  include_directories: inc,
  dependencies: [libpkgreconcile_dep, libpkgapply_dep],
)

meson.override_dependency('libpkgreconcile-apply', libpkgreconcile_apply_dep)

install_headers(
  '../include/libpkgreconcile-apply/export.h',
  '../include/libpkgreconcile-apply/adapter.h',
  '../include/libpkgreconcile-apply/libpkgreconcile-apply.h',
  subdir: 'libpkgreconcile-apply',
)

pkgconf.generate(
  pkg_reconcile_apply,
  name: 'libpkgreconcile-apply',
  description: 'Completed application to reconciliation projection',
  version: meson.project_version(),
  requires: ['libpkgreconcile >= 0.3.0', 'libpkgapply >= 3.0.1'],
)
