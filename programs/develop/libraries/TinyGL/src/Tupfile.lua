if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
INCLUDES = INCLUDES .. " -I../include"
compile_gcc("*.c")
tup.rule(OBJS, "kos32-ar rcs %o %f", {"../lib/libTinyGL.a", "../<>"})
