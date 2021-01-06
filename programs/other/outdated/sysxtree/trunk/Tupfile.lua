if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("sysxtree.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "sysxtree")
