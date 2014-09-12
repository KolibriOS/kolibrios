if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_menuetlibc.lua")
tup.include(HELPERDIR .. "/use_tinygl.lua")
compile_gcc{"main.cpp", "fps.cpp"}
link_gcc("cubeline")
