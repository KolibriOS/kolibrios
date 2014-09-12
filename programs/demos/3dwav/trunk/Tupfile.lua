if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("3dwav.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "3dwav")
