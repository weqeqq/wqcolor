project(
  'wqcolor',
  'cpp',
  meson_version: '>=1.3.0',
  version: '0.1.0',
  default_options: ['cpp_std=c++23,c++latest', 'warning_level=3'],
)

is_top_level = meson.is_subproject() == false
wqcolor_includes = include_directories('headers', 'sources')
wqcolor_public_includes = include_directories('headers')
wqcolor_public_args = []

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
  hwy_dep = dependency(
    'libhwy',
    fallback: ['highway', 'hwy_dep'],
    default_options: [
      'contrib=disabled',
      'examples=disabled',
      'test_standalone=true',
      'tests=disabled',
    ],
    required: true,
  )
  dependencies += [hwy_dep]
endif

compile_flags += ['-DWQCOLOR_SIMD=' + (opt_simd ? '1' : '0')]
compile_flags += ['-DWQCOLOR_DEBUG=' + (get_option('debug') ? '1' : '0')]
wqcolor_public_args += compile_flags

if get_option('default_library') == 'static'
  wqcolor_public_args += ['-DWQCOLOR_STATIC_DEFINE']
endif

wqcolor_lib = library(
  'wqcolor',
  'sources/color.cc',
  include_directories: wqcolor_includes,
  dependencies: dependencies,
  cpp_args: compile_flags,
  cpp_shared_args: ['-DWQCOLOR_EXPORTS'],
  cpp_static_args: ['-DWQCOLOR_STATIC_DEFINE'],
  gnu_symbol_visibility: 'hidden',
  install: opt_install,
)

wqcolor_dep = declare_dependency(
  link_with: wqcolor_lib,
  include_directories: wqcolor_public_includes,
  compile_args: wqcolor_public_args,
)

if opt_install
  install_headers('headers/weqeqq/color.h', subdir: 'weqeqq')
  install_headers('headers/weqeqq/color/config.h', subdir: 'weqeqq/color')
  install_headers('headers/weqeqq/color/export.h', subdir: 'weqeqq/color')
  pkgconfig = import('pkgconfig')
  pkgconfig.generate(
    wqcolor_lib,
    name: 'wqcolor',
    description: 'A high-performance color manipulation library with SIMD support',
    extra_cflags: wqcolor_public_args,
    subdirs: 'weqeqq',
    version: meson.project_version(),
  )
endif

if opt_tests
  subdir('tests')
endif
