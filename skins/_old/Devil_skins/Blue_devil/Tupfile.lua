if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Blue_devil.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Blue_devil.skn")
