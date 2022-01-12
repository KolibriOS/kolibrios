if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"
CFLAGS  = "-I../../../develop/ktcc/trunk/libc.obj/include"
LDFLAGS = "-nostdlib ../../../develop/ktcc/trunk/bin/lib/crt0.o -L../../../develop/ktcc/trunk/bin/lib"
LIBS = "-ltcc -lc.obj"

COMMAND=string.format("%s %s %s %s %s", TCC, CFLAGS, LDFLAGS , "%f -o %o",  LIBS)

tup.rule({"main.c"}, COMMAND .. tup.getconfig("KPACK_CMD"), "kmatrix")