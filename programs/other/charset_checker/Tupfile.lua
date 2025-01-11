if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("charset_checker.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "charchck")
