if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("libgfx.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "libgfx.obj")
