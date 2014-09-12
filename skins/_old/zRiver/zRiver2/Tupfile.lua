if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("zRiver2.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "zRiver2.skn")
