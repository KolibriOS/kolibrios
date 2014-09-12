if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Organics.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Organics.skn")
