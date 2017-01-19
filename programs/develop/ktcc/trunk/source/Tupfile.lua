if tup.getconfig("NO_GCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
CFLAGS = "-U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32"
compile_gcc{"tcc.c"}
compile_gcc{"libtcc.c"}
link_gcc("tcc")
