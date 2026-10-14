if tup.getconfig('NO_GCC') ~= "" then return end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_menuetlibc.lua")
compile_gcc{"main.c"}
link_gcc("binclock")
