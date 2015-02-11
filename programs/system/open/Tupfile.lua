if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("open.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "open")
