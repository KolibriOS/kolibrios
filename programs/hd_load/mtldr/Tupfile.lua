if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("mtldr.asm", "fasm %f %o", "mtldr")
