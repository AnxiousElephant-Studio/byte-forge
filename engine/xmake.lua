---@diagnostic disable: param-type-not-match

add_requires("libsdl3", { configs = { shared = true } })
add_requires("glad", { configs = { profile = "core", gl_version = "4.6" } })
add_requires("lua")
add_requires("stb")
add_requires("miniaudio")
add_requires("nlohmann_json")

target("byte-forge-engine")
    set_kind("shared")

    add_defines("BF_ENGINE_BUILD")
    add_includedirs("include", { public = true })
    add_includedirs("source")

    if is_plat("windows") then
        add_defines("_CRT_SECURE_NO_WARNINGS", { public = true })
    end

    add_headerfiles("include/(byte-forge-engine/**.h)")
    add_files("source/**.cpp")

    add_packages("libsdl3")
    add_packages("glad")
    add_packages("lua")
    add_packages("stb")
    add_packages("miniaudio")
    add_packages("nlohmann_json")
