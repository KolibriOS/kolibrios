if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("zRiver6.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "zRiver6.skn")
