if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Win10.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Win10.skn")
