if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"

CFLAGS  = "-I../../develop/libraries/kolibri-libc/include -I../../develop/ktcc/trunk/libc/include"
LDFLAGS = "-nostdlib ../../develop/libraries/kolibri-libc/lib/crt0.o -L../../develop/libraries/kolibri-libc/lib -L../../develop/ktcc/trunk/bin/lib"
LIBS = "-lcryptal -ldialog -lbox -lc.obj"

COMMAND=string.format("%s %s %s %s %s ", TCC, CFLAGS, "%f -o %o", LDFLAGS, LIBS)
tup.rule("thashview.c", COMMAND .. tup.getconfig("KPACK_CMD"), "thashview")
