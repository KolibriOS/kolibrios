if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("bootsect.asm", "fasm %f %o ", "bootsect.bin")
