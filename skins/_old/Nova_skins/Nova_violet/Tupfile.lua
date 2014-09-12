if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Nova_violet.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Nova_violet.skn")
