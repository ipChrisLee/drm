add_rules("mode.debug", "mode.release")
add_requires("conan::cxxopts/3.3.1")
add_requires("conan::yaml-cpp/0.8.0")
add_requires("conan::abseil/20250127.0")
add_requires("conan::tl-expected/20190710")
add_requires("conan::magic_enum/0.9.7")
add_requires("conan::dbg-macro/0.5.1")
add_requires("conan::libassert/2.1.4")
add_requires("conan::fmt/11.2.0")
add_requires("conan::range-v3/0.12.0")

target("drm")
set_kind("binary")
add_files("src/cpp/*.cpp")
set_languages("c++17")
add_cxflags("-Wall", "-Werror")
add_packages("conan::cxxopts/3.3.1")
add_packages("conan::yaml-cpp/0.8.0")
add_packages("conan::abseil/20250127.0")
add_packages("conan::tl-expected/20190710")
add_packages("conan::magic_enum/0.9.7")
add_packages("conan::dbg-macro/0.5.1")
add_packages("conan::libassert/2.1.4")
add_packages("conan::fmt/11.2.0")
add_packages("conan::range-v3/0.12.0")
add_includedirs("src/include")

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro definition
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--
