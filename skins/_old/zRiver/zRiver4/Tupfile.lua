if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("zRiver4.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "zRiver4.skn")
