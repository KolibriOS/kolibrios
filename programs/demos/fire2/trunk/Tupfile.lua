if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("fire2.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "fire2")
