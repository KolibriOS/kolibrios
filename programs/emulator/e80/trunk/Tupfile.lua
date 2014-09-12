if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
LDFLAGS = LDFLAGS .. " -T kolibri.ld"
if tup.getconfig("LANG") == "ru"
then C_LANG = "LANG_RUS"
else C_LANG = "LANG_ENG" -- this includes default case without config
end

if tup.getconfig("TUP_PLATFORM") == "win32"
-- on win32 '#' is not a special character, but backslash and quotes would be printed as is
then tup.rule('echo #define ' .. C_LANG .. ' 1 > %o', {"lang.h"})
-- on unix '#' should be escaped
else tup.rule('echo "#define" ' .. C_LANG .. ' 1 > %o', {"lang.h"})
end
tup.append_table(OBJS, tup.rule("asm_code.asm", "fasm %f %o", "asm_code.obj"))
compile_gcc{"z80/z80.c", "system/kolibri.c", "system/stdlib.c", "system/string.c", "e80.c", extra_inputs = {"lang.h"}}
link_gcc("e80")
