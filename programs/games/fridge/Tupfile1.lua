if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = "-c -fno-ident -O2 -fomit-frame-pointer -fno-ident -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -std=c99"

-- C_Layer
CFLAGS = CFLAGS .. " -I ../../../contrib/C_Layer/INCLUDE"
LDFLAGS = LDFLAGS .. " ../../../contrib/C_Layer/OBJ/loadlibimg.obj"

compile_gcc{"fridge.c"}
link_gcc("fridge")
