if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Fever_green.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Fever1 green.skn")
