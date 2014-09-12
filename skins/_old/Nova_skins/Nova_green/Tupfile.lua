if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Nova_green.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Nova_green.skn")
