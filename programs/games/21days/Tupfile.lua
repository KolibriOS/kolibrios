if tup.getconfig("NO_GCC") ~= "" then return end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_menuetlibc.lua")
CFLAGS = CFLAGS .. " -Wno-write-strings -D _KOS32 "
compile_gcc{"main.cpp", "game.cpp", "interface.cpp", "sys.cpp"}
link_gcc("21days")
