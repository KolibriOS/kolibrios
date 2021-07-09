if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("kordldr.win.asm", "fasm %f %o", "kordldr.win.bin")
