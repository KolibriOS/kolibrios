if tup.getconfig("NO_TCC") ~= "" then return end

TCC="../../develop/ktcc/trunk/bin/kos32-tcc"

CFLAGS  = "-I../../develop/ktcc/trunk/libc.obj/include"

LIBS = "-ltcc -lnetwork -lc.obj"

COMMAND=string.format("%s %s %s %s", TCC, CFLAGS, "%f -o %o",  LIBS)
tup.rule("whois.c", COMMAND .. tup.getconfig("KPACK_CMD"), "whois")
