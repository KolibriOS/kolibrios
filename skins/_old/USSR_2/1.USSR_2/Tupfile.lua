if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("1.USSR_2.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "USSR2.skn")
