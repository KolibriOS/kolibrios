if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

LDFLAGS = " -shared -s -T dll.lds --entry _DllStartup --image-base=0 --out-implib ../../lib/libsqlite3.dll.a -L../../lib "

CFLAGS = CFLAGS .. " -U__MINGW32__ -UWIN32 -UWindows -U_WINDOWS -U_WIN32 -U__WIN32__ -U_WIN32 -DPACKAGE_NAME=\"sqlite\" -DPACKAGE_TARNAME=\"sqlite\" -DPACKAGE_VERSION=\"3.36.0\" -DPACKAGE_STRING=\"sqlite-3.36.0\" -DPACKAGE=\"sqlite\" -DVERSION=\"3.36.0\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=0 -DHAVE_DLFCN_H=0  -DHAVE_FDATASYNC=0 -DHAVE_USLEEP=0 -DHAVE_LOCALTIME_R=1 -DHAVE_GMTIME_R=1 -DHAVE_DECL_STRERROR_R=1 -DHAVE_STRERROR_R=1 -DHAVE_EDITLINE_READLINE_H=1 -DHAVE_EDITLINE=1 -DHAVE_ZLIB_H=1 -I. -D_REENTRANT=1 -DSQLITE_THREADSAFE=0 -DSQLITE_ENABLE_MATH_FUNCTIONS -DSQLITE_ENABLE_FTS4 -DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_JSON1 -DSQLITE_ENABLE_RTREE -DSQLITE_ENABLE_GEOPOLY -DSQLITE_HAVE_ZLIB=1 -DSQLITE_OS_OTHER=1 -USQLITE_OS_WIN_H -DSQLITE_TEMP_STORE=3 -D_NO_STDERR "


compile_gcc{"sqlite3.c", "kolibri_vfs.c"}

OBJS.extra_inputs = {"../../lib/<libc.dll.a>", "../../lib/<libdll.a>"}

tup.rule(OBJS, "kos32-ld sqlite3.def" .. LDFLAGS ..  "-o %o %f -lgcc -lc.dll -ldll " .. tup.getconfig("KPACK_CMD"),
  {"../../bin/sqlite3.dll", extra_outputs = {"../../lib/libsqlite3.dll.a", "../../lib/<libsqlite3.dll.a>"}})
