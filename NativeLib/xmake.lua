if is_mode("debug") then 
    set_symbols("debug")
    add_defines("DEBUG")
    set_optimize("none")
end

if is_mode("release") then 
    set_symbols("hidden")
    set_optimize("fastest")
    set_strip("all")
end 

set_arch("x64")
if is_host("windows") then
    set_plat("mingw")
elseif is_host("linux") then
    set_plat("linux")
    set_symbols("hidden")
end

add_requires("glfw 3.4", {configs = {shared = true}})   -- 必须使用动态库，共享 GLFW 的状态
add_requires("vulkansdk")

set_languages("c11")
set_warnings("all", "error")


target("nativelib_windowing")
    set_kind("shared")
    set_prefixname("")

    add_files("src/nativelib_windowing/**.c", "src/common/**.c")

    add_packages("glfw")
target_end()


target("nativelib_vkrenderer")
    set_kind("shared")
    set_prefixname("")

    add_files("src/nativelib_vkrenderer/**.c", "src/nativelib_vkrenderer/**.cpp", "src/common/**.c")

    add_cxxflags("-Wno-unused-variable")

    add_packages("vulkansdk")
target_end()