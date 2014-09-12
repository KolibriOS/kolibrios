if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Red_devil.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Red_devil.skn")
