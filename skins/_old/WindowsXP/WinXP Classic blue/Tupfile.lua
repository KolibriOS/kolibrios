if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("WinXP_Classic_blue.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "WinXP Classic blue.skn")
