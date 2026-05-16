if tup.getconfig("NO_TCC") ~= "" then return end


HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_tcc.lua")

LIBC_INCLUDE = "../../libc.obj/include"

CFLAGS += "-c -I. -I" .. LIBC_INCLUDE

SOURCES = {
    "*.c",
    extra_inputs = {
        LIBC_INCLUDE .. "/shell_api.h",
        LIBC_INCLUDE .. "/sys/ksys.h",
        LIBC_INCLUDE .. "/string.h",
        LIBC_INCLUDE .. "/stdlib.h",
        LIBC_INCLUDE .. "/stdio.h"
    }
}

compile_tcc(SOURCES)

tup.rule(OBJS, "ar -rcs %o %f", "libshell.a")
