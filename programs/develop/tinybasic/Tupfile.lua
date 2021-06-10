if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"

CFLAGS  = "-I../ktcc/trunk/libc/include"
LDFLAGS = "-nostdlib ../ktcc/trunk/bin/lib/crt0.o -L../ktcc/trunk/bin/lib"
LIBS = "-lck"

COMMAND=string.format("%s %s %s %s %s ", TCC, CFLAGS, "%f -o %o", LDFLAGS, LIBS)
tup.rule("TinyBasic.c", COMMAND .. tup.getconfig("KPACK_CMD"), "TinyBasic")
