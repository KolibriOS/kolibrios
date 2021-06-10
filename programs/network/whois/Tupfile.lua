if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"

CFLAGS  = "-I../../develop/ktcc/trunk/libc.obj/include"
LDFAGS  = "-nostdlib -L../../develop/ktcc/trunk/bin/lib ../../develop/ktcc/trunk/bin/lib/crt0.o"

LIBS = "-ltcc -lnetwork -lc.obj"

COMMAND=string.format("%s %s %s %s %s", TCC, CFLAGS, LDFAGS, "%f -o %o",  LIBS)
tup.rule("whois.c", COMMAND .. tup.getconfig("KPACK_CMD"), "whois")
