if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
tup.include(HELPERDIR .. "/use_sound.lua")

CFLAGS = CFLAGS .. " -std=c99"
INCLUDES = INCLUDES .. " -I../../../contrib/sdk/sources/SDL-1.2.2_newlib/include"
table.insert(LIBDEPS, "../../../contrib/sdk/lib/<libSDLn>")
LIBS = "-lSDLn " .. LIBS

-- Subsystem native
LDFLAGS = LDFLAGS .. " --subsystem native"

compile_gcc{"SDLTest.c"}
link_gcc("SDLTest")
