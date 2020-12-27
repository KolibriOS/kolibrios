if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")
 
CFLAGS = CFLAGS .. " -std=c99"
INCLUDES = INCLUDES .. " -I ."
 
compile_gcc{"syn-att.c", "decode.c", "syn.c", "itab.c", "syn-intel.c", "udis86.c", "input.c"}
tup.rule(OBJS, "kos32-ar rcs %o %f", {"libudis86.a", "<libudis86>"})
