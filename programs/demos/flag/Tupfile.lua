if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("flag.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "flag")
