if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("charsets.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "charsets")
