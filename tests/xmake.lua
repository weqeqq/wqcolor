target("module_check")
set_kind("binary")
set_default(false)
set_policy("build.c++.modules", true)

add_files("module_check.cc")

add_deps("weqeqq.color")
add_packages("weqeqq.parallel")

add_tests("default")

target("simd_check")
set_kind("binary")
set_default(false)

add_files("simd_check.cc")

add_deps("weqeqq.color.simd")
add_packages("weqeqq.parallel")

add_tests("default")
