if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("libimg.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "libimg.obj")
