if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("personal.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "personal")
