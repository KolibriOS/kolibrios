if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("MenuetOS.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "1.MenuetOS.2.skn")
