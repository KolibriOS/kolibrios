if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("BrownLustre.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "BrownLustre.skn")
