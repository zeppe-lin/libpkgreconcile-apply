# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later
project(
  'libpkgreconcile-apply', 'cpp',
  version: '0.1.1',
  license: 'GPL-3.0-or-later',
  meson_version: '>=1.6.0',
  default_options: ['cpp_std=c++17', 'warning_level=3', 'default_library=shared', 'b_ndebug=if-release'],
)
library_kind = get_option('default_library')
if library_kind == 'both'
  error('libpkgreconcile-apply requires separate shared and static builds')
endif
want_static = get_option('link_mode') == 'static'
if (library_kind == 'static') != want_static
  error('default_library and link_mode must both select shared or static')
endif
cpp = meson.get_compiler('cpp')
visibility_cpp_args = []
if cpp.has_argument('-fvisibility-inlines-hidden')
  visibility_cpp_args += ['-fvisibility-inlines-hidden']
endif
pkgconf = import('pkgconfig')
libpkgreconcile_dep = dependency('libpkgreconcile', version: '>=0.3.0', required: true, static: want_static)
libpkgapply_dep = dependency('libpkgapply', version: '>=3.0.1', required: true, static: want_static)
subdir('src')
subdir('man')
subdir('tests')
