if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = CFLAGS .. " -DHAVE_UNISTD_H -U_Win32 -U_WIN32 -U__MINGW32__"

compile_gcc{"adler32.c", "compress.c", "crc32.c", "deflate.c", "gzclose.c", "gzlib.c", "gzread.c", "gzwrite.c", "infback.c", "inffast.c", "inflate.c", "inftrees.c", "trees.c", "uncompr.c", "zutil.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../../lib/libz.a", "../../lib/<libz>"})
