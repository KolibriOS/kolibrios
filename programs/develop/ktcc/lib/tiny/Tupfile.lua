if tup.getconfig("NO_FASM") ~= "" then return end

HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")

FASM_SOURCES = {
    "tiny.asm",
}

tup.rule(FASM_SOURCES, FASM .. " %f %o", "%B.o")
