if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("console.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "console.obj")
