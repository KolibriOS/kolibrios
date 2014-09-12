if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("WinXP_Classic_orange.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "WinXP Classic orange.skn")
