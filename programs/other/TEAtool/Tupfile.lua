if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"

CFLAGS  = "-I../../develop/ktcc/trunk/libc/include"
LDFLAGS = "-nostdlib ../../develop/ktcc/trunk/bin/lib/crt0.o -L../../develop/ktcc/trunk/bin/lib"
LIBS = "-lck"

COMMAND=string.format("%s %s %s %s %s ", TCC, CFLAGS, "%f -o %o", LDFLAGS, LIBS)
tup.rule("teatool.c", COMMAND .. tup.getconfig("KPACK_CMD"), "teatool")
