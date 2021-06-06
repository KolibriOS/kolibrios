if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

LDFLAGS = " -shared -s -T dll.lds --entry _DllStartup --image-base=0 --out-implib ../../lib/libz.dll.a -L../../lib "

CFLAGS = CFLAGS .. " -DHAVE_UNISTD_H -U_Win32 -U_WIN32 -U__MINGW32__"

compile_gcc{"adler32.c", "compress.c", "crc32.c", "deflate.c", "gzclose.c", "gzlib.c", "gzread.c", "gzwrite.c", "infback.c", "inffast.c", "inflate.c", "inftrees.c", "trees.c", "uncompr.c", "zutil.c"}
--tup.rule(OBJS, "kos32-ar rcs %o %f", {"../../lib/libz.a", "../../lib/<libz>"})

OBJS.extra_inputs = {"../../lib/<libc.dll.a>", "../../lib/<libdll.a>"}

tup.rule(OBJS, "kos32-ld zlib.def" .. LDFLAGS ..  "-o %o %f -lgcc -lc.dll -ldll " .. tup.getconfig("KPACK_CMD"),
  {"../../bin/libz.dll", extra_outputs = {"../../lib/libz.dll.a", "../../lib/<libz.dll.a>"}})
