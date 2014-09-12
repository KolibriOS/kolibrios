if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Orange_Silence.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Orange_Silence.skn")
