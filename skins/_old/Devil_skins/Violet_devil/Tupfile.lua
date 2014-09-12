if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Violet_devil.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Violet_devil.skn")
