if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("sort.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "sort.obj")
