if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("pasta.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "pasta")
