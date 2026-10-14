if tup.getconfig("NO_FASM") ~= "" then return end

tup.include(HELPERDIR .. "/use_fasm.lua")

FASM_SOURCES = {
    "tiny.asm",
}

tup.rule(FASM_SOURCES, FASM .. " %f %o", "%B.o")
