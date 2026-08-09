# SPDX-FileCopyrightText: 2026 Alexandr Savca
# SPDX-License-Identifier: GPL-3.0-or-later

if get_option('man_pages').allowed()
  scdoc = find_program('scdoc', required: get_option('man_pages'))
  if scdoc.found()
    custom_target(
      'libpkgreconcile-apply.3',
      input: 'libpkgreconcile-apply.3.scdoc',
      output: 'libpkgreconcile-apply.3',
      command: [scdoc],
      feed: true,
      capture: true,
      install: true,
      install_dir: get_option('mandir') / 'man3',
    )
  endif
endif
