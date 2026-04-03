project(
  'wqcolor',
  'cpp',
  version: '0.1.0',
  default_options: ['cpp_std=c++23,c++latest', 'warning_level=3'],
)

is_top_level = meson.is_subproject() == false

opt_tests = get_option('tests').disable_auto_if(not is_top_level).enabled()
opt_install = get_option('install').disable_auto_if(not is_top_level).enabled()
opt_simd = get_option('simd').enabled()

wqparallel_dep = dependency(
  'wqparallel',
  version: '>=0.2.1',
  fallback: ['wqparallel', 'wqparallel_dep'],
  default_options: ['tests=disabled'],
  required: true,
)

dependencies = [wqparallel_dep]
compile_flags = []

if opt_simd
  hwy_dep = dependency('libhwy', required: true)
  dependencies += [hwy_dep]
endif

compile_flags += ['-DWQCOLOR_SIMD=' + (opt_simd ? '1' : '0')]
compile_flags += ['-DWQCOLOR_DEBUG=' + (get_option('debug') ? '1' : '0')]

wqcolor_lib = library(
  'wqcolor',
  'sources/color.cc',
  include_directories: ['headers', 'sources'],
  dependencies: dependencies,
  cpp_args: compile_flags,
  install: opt_install,
)

wqcolor_dep = declare_dependency(
  link_with: wqcolor_lib,
  include_directories: include_directories('headers'),
  compile_args: compile_flags,
)

if opt_install
  install_headers('headers/weqeqq/color.h', subdir: 'weqeqq')
  install_headers('headers/weqeqq/color/config.h', subdir: 'weqeqq/color')
  pkgconfig = import('pkgconfig')
  pkgconfig.generate(
    wqcolor_lib,
    name: 'wqcolor',
    description: 'A high-performance color manipulation library with SIMD support',
    subdirs: 'weqeqq',
    version: meson.project_version(),
  )
endif

if opt_tests
  subdir('tests')
endif
