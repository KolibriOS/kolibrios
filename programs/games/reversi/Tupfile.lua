if tup.getconfig('NO_GCC') ~= "" then return end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_menuetlibc.lua")
compile_gcc{"reversi.c"}
link_gcc("reversi")
