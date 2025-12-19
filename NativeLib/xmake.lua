add_rules("mode.debug")
add_defines("DEBUG")

set_arch("x64")
if is_host("windows") then
    set_plat("mingw")
elseif is_host("linux") then
    set_plat("linux")
    set_symbols("hidden")
end

add_requires("glfw 3.4", {configs = {shared = true}})   -- 必须使用动态库，共享 GLFW 的状态
add_requires("vulkansdk")

set_languages("c99")
set_warnings("all", "error")


target("nativelib_windowing")
    set_kind("shared")
    set_prefixname("")

    add_files("src/nativelib_windowing.c")

    add_packages("glfw")
target_end()


target("nativelib_vulkan_renderer")
    set_kind("shared")
    set_prefixname("")

    add_files("src/nativelib_vulkan_renderer.c")

    add_packages("vulkansdk", "glfw")
target_end()