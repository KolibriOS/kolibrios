if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("libini.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "libini.obj")
