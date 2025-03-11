if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("tube.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "tube")
