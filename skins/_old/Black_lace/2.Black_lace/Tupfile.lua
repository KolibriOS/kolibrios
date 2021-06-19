if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("2.Black_lace.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Black_lace.skn")
