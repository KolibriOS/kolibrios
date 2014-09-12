if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("3.Circles.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "3.Circles.skn")
