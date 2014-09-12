if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Nova_brown.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Nova_brown.skn")
