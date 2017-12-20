if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_MSVC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.include(HELPERDIR .. "/use_msvc.lua")

compile_msvc{"ac97snd/ac97wav.c", "ac97snd/crt.c"}
OBJS += tup.foreach_rule({"ac97snd/k_lib.asm", "ufmod-config.asm"}, FASM .. " %f %o", "%B.obj")
OBJS += "mpg/mpg.lib"
OBJS += "../../develop/sdk/trunk/sound/src/sound.lib"
link_msvc("ac97snd.bin")
