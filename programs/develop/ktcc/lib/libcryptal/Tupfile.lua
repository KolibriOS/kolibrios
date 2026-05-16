if tup.getconfig("NO_TCC") ~= "" then return end

HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_tcc.lua")

LIBC_INCLUDE = "../../libc.obj/include"

CFLAGS += "-c -I. -I" .. LIBC_INCLUDE

SOURCES = {
    "*.c",
    extra_inputs = {
        "memory.h",
        LIBC_INCLUDE .. "/cryptal/*",
        LIBC_INCLUDE .. "/string.h",
        LIBC_INCLUDE .. "/stdlib.h",
        LIBC_INCLUDE .. "/stdio.h",
    }
}

compile_tcc(SOURCES)

tup.rule(OBJS, "ar -rcs %o %f", "libcryptal.a")
