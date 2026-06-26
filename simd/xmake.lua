add_repositories("weqeqq.repo https://github.com/weqeqq/xmake-repo.git")
add_requires("weqeqq.parallel")

option("enable-simd")
set_default(false)
set_description("Enable SIMD")
option_end()

if has_config("enable-simd") and is_config("enable-simd", true) then
	add_requires("highway")
end

target("weqeqq.color.simd")
-- Object library: its objects fold directly into weqeqq.color's archive, so
-- there is no second static lib to link (and no link-order pitfalls).
set_kind("object")
set_policy("build.c++.modules", true)

add_files("sources/**.cpp")
add_includedirs("sources")
add_includedirs("headers", { public = true })

add_packages("weqeqq.parallel")

if is_mode("debug") then
	add_defines("WQCOLOR_DEBUG=1")
end

if is_kind("static") then
	add_defines("WQCOLOR_STATIC_DEFINE")
end

if has_config("enable-simd") and is_config("enable-simd", true) then
	add_defines("WQCOLOR_SIMD")
	add_packages("highway")
end
