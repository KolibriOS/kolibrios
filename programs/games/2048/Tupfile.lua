if tup.getconfig('NO_GCC') ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_menuetlibc.lua")
compile_gcc{"main.c" "defines.c" "rect.c" "cell.c" "board.c" "game.c"}
link_gcc("2048")
