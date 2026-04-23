if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("box_lib.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "box_lib.obj")
