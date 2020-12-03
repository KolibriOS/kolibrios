if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

-- C_Layer
CFLAGS = CFLAGS .. " -std=c99 -I ../../../contrib/C_Layer/INCLUDE"
LDFLAGS = LDFLAGS .. " ../../../contrib/C_Layer/OBJ/loadlibimg.obj"

compile_gcc{"fridge.c"}
link_gcc("fridge")
