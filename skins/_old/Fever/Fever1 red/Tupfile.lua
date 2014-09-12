if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Fever_red.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Fever1 red.skn")
