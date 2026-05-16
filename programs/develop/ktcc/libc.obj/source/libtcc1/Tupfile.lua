if tup.getconfig("NO_TCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end

local helperPath = "../../../../.."
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and helperPath or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_tcc.lua")
tup.include(HELPERDIR .. "/use_fasm.lua")

TCC_SOURCES = {
    "libtcc1.c",
}

FASM_SOURCES = {
    "memcpy.asm",
    "memmove.asm",
    "memset.asm",
    extra_inputs = {
        helperPath .. "/proc32.inc"
    }
}

compile_tcc(TCC_SOURCES)

tup.append_table(OBJS, tup.foreach_rule(FASM_SOURCES, FASM .. " %f %o", "%B.o"))

tup.rule(OBJS, "ar -rcs %o %f", "libtcc1.a")
