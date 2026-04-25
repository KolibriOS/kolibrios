if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("utils.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "utils.obj")
