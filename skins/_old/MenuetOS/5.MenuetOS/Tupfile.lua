if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("MenuetOS.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "MenuetOS 5.skn")
