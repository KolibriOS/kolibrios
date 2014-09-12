if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Nova_blue.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Nova_blue.skn")
