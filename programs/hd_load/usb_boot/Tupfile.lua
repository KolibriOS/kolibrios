if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("setmbr.asm", "fasm %f %o", "setmbr.exe")
tup.rule("inst.asm", "fasm %f %o", "inst.exe")
tup.rule("BOOT_F32.ASM", "fasm %f %o", "BOOT_F32.BIN")
tup.rule("mtldr.asm", "fasm %f %o", "MTLD_F32")
