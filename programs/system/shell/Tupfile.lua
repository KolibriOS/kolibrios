if tup.getconfig("NO_TCC") ~= "" then return end

HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_tcc.lua")

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

LIBS = ""

link_tcc({"shell.c", "system/kolibri.c", extra_inputs = {"lang.h"}}, "shell"); 
