if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_sound.lua")
LDFLAGS = LDFLAGS .. " -T kolibri.ld"
tup.append_table(OBJS, tup.rule("start.asm", "fasm %f %o", "start.o"))
compile_gcc{"system/kolibri.c", "game/rs/rsmicrolibc.c", "game/rs/rsplatform_kos.c", "game/rs/rsmx.c", "game/rsnoise.c", "game/rsgentex.c", "game/rsmain.c",  "game/rsgame.c", "game/rsgamedraw.c", "game/rskos.c", "game/rsgametext.c", "game/rsgamemenu.c"}
link_gcc("helio")
