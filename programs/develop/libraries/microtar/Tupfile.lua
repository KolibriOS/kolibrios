if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")

CFLAGS = " -c -w -nostdinc -DGNUC -DMTAR_OBJ -Os -fno-common -fno-builtin -fno-leading-underscore -fno-pie" 
INCLUDES = " -I../include -I../../ktcc/trunk/libc.obj/include"

tup.rule("microtar.c", "kos32-gcc" .. CFLAGS .. INCLUDES .. " -o %o %f " .. tup.getconfig("KPACK_CMD"), "mtar.obj")
