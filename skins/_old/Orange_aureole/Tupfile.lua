if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("orange_aureole.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Orange_aureole.skn")
