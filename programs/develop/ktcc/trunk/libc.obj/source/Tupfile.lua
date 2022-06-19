if tup.getconfig("NO_TCC") ~= "" then return end

CFLAGS = " -r -nostdinc -nostdlib -DGNUC -D_BUILD_LIBC "
INCLUDES = " -I../include"

GAS_SRC = {
    "setjmp/setjmp.s",
    "setjmp/longjmp.s",
    "math/round.s",
    "math/atan.s",
    "math/pow2.s",
    "math/log10.s",
    "math/exp.s",
    "math/pow10.s",
    "math/log.s",
    "math/pow.s",
    "math/ceil.s",
    "math/cos.s",
    "math/sin.s",
    "math/asin.s",
    "math/modf.s",
    "math/floor.s",
    "math/fabs.s",
    "math/fmod.s",
    "math/log2.s",
    "math/acos.s",
    "math/modfl.s",
    "math/atan2.s",
    "math/sqrt.s",
    "math/tan.s",
    "string/memset.s",
    "string/memmove.s"
}

OBJS = {"libc.c"}

tup.append_table(OBJS,
  tup.foreach_rule(GAS_SRC, "as --32 %f -o %o", "%B.o")
)

tup.rule(OBJS, "kos32-tcc" .. CFLAGS .. INCLUDES .. " %f -o %o " .. " && strip %o --strip-unneeded " , "libc.o")
tup.rule("libc.o", "objconv -fcoff32 %f %o " .. tup.getconfig("KPACK_CMD"), "%B.obj")
