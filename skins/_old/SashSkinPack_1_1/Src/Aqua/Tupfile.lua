if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Aqua.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Aqua.skn")
