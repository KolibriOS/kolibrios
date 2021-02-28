if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

SDK_DIR = "../../../sdk"

CFLAGS = CFLAGS .. " -std=c99 -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32"
INCLUDES = INCLUDES .. " -I ."

compile_gcc{"bio.c", "cio.c", "dwt.c", "event.c", "image.c", "j2k.c", "j2k_lib.c", "jp2.c", "jpt.c", "lrintf.c", "mct.c", "mqc.c", "openjpeg.c", "pi.c", "raw.c", "t1.c", "t1_generate_luts.c", "t2.c", "tcd.c", "tgt.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../../lib/libopenjpeg.a", "<../../lib/libopenjpeg>"})
