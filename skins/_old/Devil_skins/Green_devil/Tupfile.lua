if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Green_devil.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Green_devil.skn")
