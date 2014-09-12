if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("1.Black_lace.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "1.Black_lace.skn")
