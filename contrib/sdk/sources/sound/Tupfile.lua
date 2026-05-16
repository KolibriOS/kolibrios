if tup.getconfig("NO_FASM") ~= "" then return end

SOURCES = {
    "src/*.asm",
    extra_inputs = {
        "src/*.inc"
    }
}

OBJS = {
    extra_inputs = {
        "symbols"
    }
}

tup.append_table(OBJS, tup.foreach_rule(SOURCES, "fasm %f %o", "%B.o"))

tup.rule(OBJS, "ar -cvrs %o %f && objcopy -O elf32-i386 --redefine-syms=symbols %o", {"libsound.a", "<libsound.a>"})
