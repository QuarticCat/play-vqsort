add_rules("mode.debug", "mode.release", "mode.releasedbg")

set_languages("c++20")

target("pvq")
    set_kind("headeronly")
    add_includedirs("include", { public = true })
    add_headerfiles("include/(pvq/**.hpp)")

for _, filepath in ipairs(os.filedirs("bench/*.cpp")) do
    local filename = filepath:match("bench/(.+).cpp$")
    target("bench:" .. filename)
        set_kind("binary")
        add_deps("pvq")
        add_files(filepath)
        add_cxxflags("-march=native")
end
