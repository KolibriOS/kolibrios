if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Default.skn.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "win8.skn")
