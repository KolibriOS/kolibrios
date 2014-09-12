if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
LDFLAGS = LDFLAGS .. " -T kolibri.ld"
tup.append_table(OBJS, tup.rule("asm_code.asm", "fasm %f %o", "asm_code.o"))
compile_gcc{"c_code.c", "system/kolibri.c", "system/stdlib.c", "system/string.c", "system/gblib.c"}
link_gcc("donkey")
