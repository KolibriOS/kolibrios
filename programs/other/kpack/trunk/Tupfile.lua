if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("kpack.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "kpack")
