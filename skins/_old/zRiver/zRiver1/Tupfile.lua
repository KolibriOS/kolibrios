if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("zRiver1.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "zRiver1.skn")
