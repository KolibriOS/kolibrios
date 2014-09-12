if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("BrownLustre_2.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "BrownLustre_2.skn")
