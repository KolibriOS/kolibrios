if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
LDFLAGS = LDFLAGS .. " -T kolibri.ld"
tup.append_table(OBJS, tup.rule("start.asm", "fasm %f %o", "start.o"))
compile_gcc{"foxhunt.c", "system/kolibri.c", "system/stdlib.c", "system/string.c", "system/ctype.c"}
link_gcc("foxhunt")
