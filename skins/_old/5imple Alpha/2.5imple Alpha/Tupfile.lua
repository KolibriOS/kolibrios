if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("2.5imple_Alpha.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Simple Alpha.skn")
