if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("9x2klbr.asm", "fasm %f %o", "9x2klbr.exe")
