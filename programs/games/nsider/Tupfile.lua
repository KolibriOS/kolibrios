if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
CFLAGS = CFLAGS .. " -I include"
LDFLAGS = LDFLAGS .. " -T kolibri.ld"
OBJS += tup.foreach_rule({"start.asm", "kolibrisys/*.asm"}, "fasm %f %o", "%B.o")
compile_gcc{"game.c"}
compile_gcc{"stdio/*.c"}
link_gcc("nsider")
