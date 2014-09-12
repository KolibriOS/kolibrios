if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("brick.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "1.Brick.skn")
