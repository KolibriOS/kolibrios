if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("bomberdata.asm", "fasm %f %o", "bomberdata.bin")
