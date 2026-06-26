set_project("weqeqq.color")
set_version("0.1.0")

add_rules("mode.release", "mode.debug")
add_rules("plugin.compile_commands.autoupdate")
set_policy("build.c++.modules.std", false)

set_languages("c++23")

add_repositories("weqeqq.repo https://github.com/weqeqq/xmake-repo.git")
add_requires("weqeqq.parallel")

option("enable-simd")
set_default(false)
set_description("Enable SIMD")
option_end()

includes("simd")
includes("tests")

target("weqeqq.color")
set_kind("$(kind)")

add_files("sources/color.cppm", { public = true })

add_headerfiles("simd/headers/(weqeqq/**.h)")

add_deps("weqeqq.color.simd")
add_packages("weqeqq.parallel")
