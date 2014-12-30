if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_GCC") ~= "" then return end
-- tup.rule("echo \"#define LANG_" .. ((tup.getconfig("LANG") == "") and "en" or tup.getconfig("LANG")) .. "\" > lang.h", {"lang.h"})
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_sound.lua")
LDFLAGS = LDFLAGS .. " -T kolibri.ld"
tup.append_table(OBJS, tup.rule("start.asm", "fasm %f %o", "start.o"))

if tup.getconfig("LANG") == "ru"
then C_LANG = "LANG_RU"

elseif tup.getconfig("LANG") == "sp"
then C_LANG = "LANG_SP" -- just for example, other languages are not implemented

else C_LANG = "LANG_EN" -- default language is English
end

CFLAGS = CFLAGS .. " -Werror=implicit -DRS_KOS -D" .. C_LANG .. " "

compile_gcc{ "system/kolibri.c", "game/rs/rsmicrolibc.c", "game/rs/rsplatform_kos.c", "game/rs/rsmx.c", "game/rsnoise.c", "game/rsgentex.c", "game/rssoundgen.c", "game/rsgame.c", "game/rsgamedraw.c", "game/rsgamelogic.c", "game/rskos.c", "game/rsgametext.c", "game/rsgamemenu.c"}
link_gcc ("heliothryx")
