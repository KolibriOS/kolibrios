if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

-- C_Layer
CFLAGS = CFLAGS .. " -std=c99 -Wall -Wextra"
INCLUDES = INCLUDES .. " -I../../../contrib/C_Layer/INCLUDE"
table.insert(LIBDEPS, "../../../contrib/C_Layer/OBJ/<C_Layer>")
LIBS = LIBS .. " ../../../contrib/C_Layer/OBJ/loadlibimg.o"

-- Subsystem native
LDFLAGS = LDFLAGS .. " --subsystem native"

compile_gcc{"fridge.c"}
link_gcc("fridge")
