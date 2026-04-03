gtest_dep = dependency('gtest', required: true)
gtest_main_dep = dependency('gtest_main', required: true)
test_deps = [wqcolor_dep, wqparallel_dep, gtest_dep, gtest_main_dep]
test_include_dirs = include_directories('headers')

test_sources = {
  'color_contract': files('sources/color_contract_test.cc'),
  'color_correctness': files('sources/color_correctness_test.cc'),
  'color_regression': files('sources/color_regression_test.cc'),
  'color_parallel': files('sources/color_parallel_test.cc'),
  'color_reference': files('sources/color_reference_test.cc'),
}

foreach test_name, test_source : test_sources
  test_exe = executable(
    test_name,
    test_source + 'sources/test_support.cc',
    include_directories: test_include_dirs,
    dependencies: test_deps
  )

  test(
    test_name,
    test_exe,
    is_parallel: false,
    protocol: 'gtest',
    suite: 'unit'
  )
endforeach
