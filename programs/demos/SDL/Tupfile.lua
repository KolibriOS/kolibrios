if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
tup.include(HELPERDIR .. "/use_sdl_newlib.lua")

CFLAGS = CFLAGS .. " -std=c99"

-- Subsystem native
LDFLAGS = LDFLAGS .. " --subsystem native"

compile_gcc{"SDLTest.c"}
link_gcc("SDLTest")
