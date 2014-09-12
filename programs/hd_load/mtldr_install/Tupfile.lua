if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("mtldr_code/mtldr.asm", "fasm %f %o", "mtldr_for_installer")
tup.rule({"mtldr_install.asm", extra_inputs = {"mtldr_for_installer"}}, "fasm %f %o", "mtldr_install.exe")
