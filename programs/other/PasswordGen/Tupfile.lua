if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"
CFLAGS  = "-I../../develop/ktcc/trunk/libc.obj/include"
LDFLAGS = "-nostdlib ../ktcc/trunk/bin/lib/crt0.o -L../ktcc/trunk/bin/lib"

LIBS = "-ltcc -lbox -lc.obj"
COMMAND=string.format("%s %s %s %s %s", TCC, CFLAGS, LFLAGS, "%f -o %o",  LIBS)

tup.rule("passwordgen.c", COMMAND .. tup.getconfig("KPACK_CMD"), "passwordgen")
