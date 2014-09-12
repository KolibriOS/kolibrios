if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("buf2d.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "buf2d.obj")
