if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("apm.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "apm")
