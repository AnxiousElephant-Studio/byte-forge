target("player")
    set_kind("binary")

    add_files("source/**.cpp")

    add_deps("byte-forge-engine")

    add_packages("libsdl3")

    after_build(function (target)
        for _, package in pairs(target:pkgs()) do
            local package_bin_directory = path.join(package:installdir(), "bin")

            if os.isdir(package_bin_directory) then
                os.trycp(path.join(package_bin_directory, "*.dll"), target:targetdir())
            end
        end
    end)
