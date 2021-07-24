if tup.getconfig("NO_FASM") ~= "" then return end
if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"

CFLAGS  = "-I../../../../ktcc/trunk/libc.obj/include -I../../include" 
LDFAGS  = "-nostdlib -L../../../../ktcc/trunk/bin/lib ../../../../ktcc/trunk/bin/lib/crt0.o -stack=10000 "

LIBS = "-ltcc -lnetwork -lc.obj "

COMMAND=string.format("%s %s %s %s %s", TCC, CFLAGS, LDFAGS, "%f -o %o",  LIBS)
tup.rule("load_mbedtls.asm", "fasm %f %o ", "load_mbedtls.o")
tup.rule({"ssl_client1.c", "load_mbedtls.o"}, COMMAND .. tup.getconfig("KPACK_CMD"), "ssl_client1")
