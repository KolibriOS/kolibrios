if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"

CFLAGS  = "-I../../develop/libraries/kolibri-libc/include -I../../develop/ktcc/trunk/libc/include"
LDFLAGS = "-nostdlib ../../develop/libraries/kolibri-libc/lib/crt0.o -L../../develop/libraries/kolibri-libc/lib -L../../develop/ktcc/trunk/bin/lib"
LIBS = "-ltcc -limg -lhttp -lc.obj"

COMMAND = string.format("%s %s %s %s %s ", TCC, CFLAGS, "%f -o %o", LDFLAGS, LIBS)
tup.rule({"weather.c", "json/json.c"}, COMMAND .. tup.getconfig("KPACK_CMD"), "weather")
