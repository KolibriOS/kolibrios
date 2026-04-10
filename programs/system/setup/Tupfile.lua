if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("setup.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "setup")
