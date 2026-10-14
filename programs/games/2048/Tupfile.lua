if tup.getconfig('NO_GCC') ~= "" then return end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_menuetlibc.lua")
compile_gcc{"main.c", "defines.c", "rect.c", "paint.c", "cell.c", "config.c", "board.c", "game.c"}
link_gcc("2048")
