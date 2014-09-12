if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("convert.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "convert.obj")
