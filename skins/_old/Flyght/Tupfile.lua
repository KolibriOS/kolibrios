if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Flyght.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Flyght.skn")
