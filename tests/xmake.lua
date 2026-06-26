target("tests")
set_kind("binary")
set_default(false)
set_policy("build.c++.modules", true)

add_files("reference.cppm")
add_files("*.cpp")

add_deps("weqeqq.color")
add_packages("weqeqq.parallel")
add_packages("weqeqq.test", { components = { "core", "main" } })

add_tests("default")
