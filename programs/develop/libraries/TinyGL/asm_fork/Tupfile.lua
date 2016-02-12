if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("tinygl.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "tinygl.obj")
