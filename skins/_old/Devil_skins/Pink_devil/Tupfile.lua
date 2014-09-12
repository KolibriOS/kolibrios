if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Pink_devil.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Pink_devil.skn")
