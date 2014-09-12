if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("russia.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Russia.skn")
