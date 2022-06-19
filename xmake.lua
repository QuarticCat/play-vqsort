add_rules("mode.debug", "mode.release", "mode.releasedbg")

set_languages("c++20")

target("pvq")
    set_kind("headeronly")
    add_includedirs("include", { public = true })
    add_headerfiles("include/(pvq/**.hpp)")
    add_cxxflags("-march=native", { public = true }) -- Should I turn off strict aliasing?

target("pvq-test")
    set_kind("binary")
    add_deps("pvq")
    add_files("test/**.cpp")

for _, filepath in ipairs(os.files("bench/*.cpp")) do
    target("bench"..path.basename(filepath))
        set_kind("binary")
        add_deps("pvq")
        add_files(filepath)
        if is_mode("debug") or is_mode("releasedbg") then
            add_cxxflags("-save-temps=obj")
        end
end
