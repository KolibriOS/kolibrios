if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("kerpack.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "kerpack")
