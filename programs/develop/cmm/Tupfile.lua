if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")

SDK_DIR = "../../../contrib/sdk"
LDFLAGS = LDFLAGS .. " -static -S -nostdlib -T" .. SDK_DIR .. "/sources/newlib/app-dynamic.lds --image-base 0 -L" .. SDK_DIR .. "/lib --stack 0x100000 "
LIBS = LIBS .. " -lgcc -ldll -lc.dll "
CFLAGS_cpp = CFLAGS_cpp .. " -Dstricmp=strcasecmp -Dstrnicmp=strncasecmp -fno-exceptions -D_KOS_ -U_Win32 -U_WIN32 -U__MINGW32__ -mno-ms-bitfields -Wno-write-strings "
INCLUDES = INCLUDES .. " -I" .. SDK_DIR .. "/sources/newlib/libc/include "

tup.append_table(LIBDEPS, {SDK_DIR .. "/lib/<libc.dll.a>"})
tup.append_table(LIBDEPS, {SDK_DIR .. "/lib/<libdll.a>"})

compile_gcc("*.cpp")
link_gcc("cmm")
