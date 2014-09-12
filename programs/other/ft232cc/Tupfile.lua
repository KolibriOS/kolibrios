if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("ft232cc.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "ft232cc")
