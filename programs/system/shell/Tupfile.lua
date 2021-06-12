if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"
CFLAGS  = "-I../../develop/ktcc/trunk/libc.obj/include"
LDFLAGS = "-nostdlib ../../develop/ktcc/trunk/bin/lib/crt0.o -L../../develop/ktcc/trunk/bin/lib"

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

LIBS = "-ltcc -lc.obj"
COMMAND=string.format("%s %s %s %s %s", TCC, CFLAGS, LDFLAGS , "%f -o %o",  LIBS)

tup.rule({"shell.c", "system/kolibri.c"}, extra_inputs = {"lang.h"}, COMMAND .. tup.getconfig("KPACK_CMD"), "shell")