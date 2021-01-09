if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
tup.include(HELPERDIR .. "/use_tinygl.lua")

LIBS = "-lstdc++ -lsupc++ " .. LIBS
LDFLAGS = LDFLAGS .. " --subsystem native"

compile_gcc{"fps.cpp", "main.cpp"}
link_gcc("gears")
