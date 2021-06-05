if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
tup.include(HELPERDIR .. "/use_sdl_newlib.lua")

SDK_DIR = "../../sdk"

CFLAGS = CFLAGS .. " -Dstricmp=strcasecmp -DZLIB_SUPPORT -Dstrnicmp=strncasecmp -DUSE_SDL -DNDEBUG -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32"
INCLUDES = INCLUDES .. " -I. -I " .. SDK_DIR .. "/sources/zlib"
LDFLAGS = LDFLAGS .. " -Lsdl --subsystem native"
LIBS = "-ls -lz.dll " .. LIBS

table.insert(LIBDEPS, SDK_DIR .. "/lib/<libz.dll.a>")
table.insert(LIBDEPS, "sdl/<libs>")

compile_gcc{"filter.c", "fireworks.c", "main.c", "menu.c", "stub.c"}
link_gcc("jumpnbump")
