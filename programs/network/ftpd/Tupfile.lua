if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("ftpd.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "ftpd")
