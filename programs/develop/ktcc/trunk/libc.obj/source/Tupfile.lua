if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../../" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")

CFLAGS = " -c -nostdinc -DGNUC -D_BUILD_LIBC -Os -fno-common -fno-builtin -fno-leading-underscore -fno-pie" 
INCLUDES = " -I../include"

tup.rule("libc.c", "kos32-gcc" .. CFLAGS .. INCLUDES .. " -o %o %f " .. tup.getconfig("KPACK_CMD"), "libc.obj")
