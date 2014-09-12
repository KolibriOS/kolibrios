if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Syllable.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Syllable.skn")
