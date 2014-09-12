if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("crt0_coff.asm", "fasm %f %o", "crt0.o")
