add_requires("imgui", { configs = { sdl3 = true, opengl3 = true } })
add_requireconfs("imgui.libsdl3", { configs = { shared = true } })

target("editor")
    set_kind("binary")

    set_rundir("$(projectdir)/applications/editor")

    add_includedirs("include")
    add_headerfiles("include/(byte-forge-editor/**.h)")
    add_files("source/**.cpp")

    add_deps("byte-forge-engine")

    add_packages("imgui", "libsdl3")

    after_build(function (target)
        for _, package in pairs(target:pkgs()) do
            local package_bin_directory = path.join(package:installdir(), "bin")

            if os.isdir(package_bin_directory) then
                os.trycp(path.join(package_bin_directory, "*.dll"), target:targetdir())
            end
        end
    end)
