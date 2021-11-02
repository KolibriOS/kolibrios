if tup.getconfig("NO_TCC") ~= "" then return end

TCC="kos32-tcc"

CFLAGS  = "-D_KOLIBRI -I../ktcc/trunk/libc.obj/include -I inc"
LDFLAGS = "-nostdlib -L../ktcc/trunk/bin/lib ../ktcc/trunk/bin/lib/crt0.o" 

LIBS = "-ltcc -lc.obj"

SRC={"src/common.c",
     "src/errors.c", 
     "src/expression.c",
     "src/formatter.c",
     "src/generatec.c",
     "src/interpret.c",
     "src/options.c",
     "src/parser.c",
     "src/statement.c",
     "src/tinybasic.c",
     "src/token.c",
     "src/tokeniser.c",
};


COMMAND=string.format("%s %s %s %s %s", TCC, CFLAGS, LDFLAGS, "%f -o %o",  LIBS)
tup.rule(SRC, COMMAND .. tup.getconfig("KPACK_CMD"), "tinybasic")
