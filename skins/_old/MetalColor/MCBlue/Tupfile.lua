if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("MCBlue.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "MCBlue.skn")
