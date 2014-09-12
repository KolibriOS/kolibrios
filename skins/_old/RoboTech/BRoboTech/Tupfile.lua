if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("BRoboTech.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "BRoboTech.skn")
