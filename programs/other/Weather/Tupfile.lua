if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"
CFLAGS  = "-I../../develop/ktcc/trunk/libc.obj/include"
LDFLAGS = "-nostdlib -L../../develop/ktcc/trunk/bin/lib ../../develop/ktcc/trunk/bin/lib/crt0.o" 

LIBS = "-ltcc -limg -lhttp -lc.obj"

COMMAND = string.format("%s %s %s %s ", TCC, CFLAGS, "%f -o %o", LIBS)
tup.rule({"weather.c", "json/json.c"}, COMMAND .. tup.getconfig("KPACK_CMD"), "weather")
