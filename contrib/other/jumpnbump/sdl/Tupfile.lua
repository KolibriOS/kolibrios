if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
tup.include(HELPERDIR .. "/use_sdl_newlib.lua")

CFLAGS = CFLAGS .. " -DNDEBUG -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -DUSE_SDL"
INCLUDES = INCLUDES .. " -I.."

compile_gcc{"gfx.c", "input.c", "interrpt.c", "sound.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"libs.a", "<libs>"})
