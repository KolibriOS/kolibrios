if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

LDFLAGS = LDFLAGS .. " --stack 0x100000"
CFLAGS = CFLAGS .. " -fpack-struct=2 -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -D_KOS_ -U_Win32 -U_WIN32 -U__MINGW32__"

compile_gcc("*.cpp")
link_gcc("cmm")
