if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("zRiver3.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "zRiver3.skn")
