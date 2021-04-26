if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"

CFLAGS  = "-I../../develop/ktcc/trunk/kolibri-libc/include"
LDFLAGS = "-nostdlib ../../develop/ktcc/trunk/kolibri-libc/crt0.o -L ../../develop/ktcc/trunk/kolibri-libc -L../../develop/ktcc/trunk/bin/lib " 
LIBS = "-lnetwork -lc.obj"

COMMAND=string.format("%s %s %s %s %s ", TCC, CFLAGS, "%f -o %o", LDFLAGS, LIBS)
tup.rule("whois.c", COMMAND .. tup.getconfig("KPACK_CMD"), "whois")
