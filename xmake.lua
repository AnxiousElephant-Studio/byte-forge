---@diagnostic disable: param-type-not-match

set_project("byte-forge")
set_version("0.1.0")

set_languages("c++20")
add_rules("mode.debug", "mode.release")

add_rules("plugin.compile_commands.autoupdate", { outputdir = "." })

includes("engine")
includes("applications/editor")
includes("applications/sandbox")
includes("applications/player")
