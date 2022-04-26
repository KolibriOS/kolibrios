if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = CFLAGS_OPTIMIZE_SPEED .. " -DOGG_MUSIC"

INCLUDES = INCLUDES .. " -I../newlib/libc/include/ -I../SDL-1.2.2_newlib/include -I../libogg-1.3.5/include -I.. -I../libvorbis-1.3.7/include "

compile_gcc{
    "effect_stereoreverse.c",
    "effect_position.c",
    "effects_internal.c",
    "music.c",
    "mixer.c",
    "load_ogg.c",
    "music_ogg.c",
    "dynamic_ogg.c",
    "wavestream.c",
    "load_aiff.c",
    "load_voc.c",
}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../../lib/libSDL_mixer.a", "../../lib/<libSDL_mixer>"})
