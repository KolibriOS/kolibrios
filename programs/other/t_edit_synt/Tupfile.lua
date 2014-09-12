if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("te_syntax.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "te_syntax")
