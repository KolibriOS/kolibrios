if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_menuetlibc.lua")
tup.include(HELPERDIR .. "/use_tinygl.lua")
compile_gcc{"fps.cpp", "main.cpp"}
link_gcc("gears")
