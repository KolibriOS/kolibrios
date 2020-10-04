if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("bootsect.asm", "fasm %f %o", "bootsect.bin")
tup.rule("kordldr.f1x.asm", "fasm %f %o", "kordldr.f1x.bin")
