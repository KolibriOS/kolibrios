if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("3.5imple_Alpha.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "3.5imple Alpha.skn")
