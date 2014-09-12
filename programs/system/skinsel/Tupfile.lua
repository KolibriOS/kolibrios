if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("skinsel.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "skinsel")
