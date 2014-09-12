if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Eyes.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Eyes.skn")
