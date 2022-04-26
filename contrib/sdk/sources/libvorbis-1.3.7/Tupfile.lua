if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = CFLAGS_OPTIMIZE_SPEED .. " -DHAVE_CONFIG "

INCLUDES = INCLUDES .. " -Iinclude -I../libogg-1.3.5/include -Ilib "

compile_gcc{
    "lib/block.c",
    "lib/sharedbook.c",
    "lib/vorbisenc.c",
    "lib/info.c",
    "lib/registry.c",
    "lib/psy.c",
    "lib/window.c",
    "lib/lpc.c",
    "lib/tone.c",
    "lib/smallft.c",
    "lib/barkmel.c",
    "lib/mdct.c",
    "lib/bitrate.c",
    "lib/analysis.c",
    "lib/vorbisfile.c",
    "lib/res0.c",
    "lib/lookup.c",
    "lib/lsp.c",
    "lib/floor1.c",
    "lib/floor0.c",
    "lib/codebook.c",
    "lib/envelope.c",
    "lib/mapping0.c",
    "lib/synthesis.c"
}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../../lib/libvorbis.a", "../../lib/<libvorbis>"})
