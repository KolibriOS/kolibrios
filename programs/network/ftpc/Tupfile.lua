if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("ftpc.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "ftpc")
