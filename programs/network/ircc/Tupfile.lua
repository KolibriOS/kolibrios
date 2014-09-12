if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("ircc.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "ircc")
