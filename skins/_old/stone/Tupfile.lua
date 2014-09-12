if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("O'stone.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "stone.skn")
