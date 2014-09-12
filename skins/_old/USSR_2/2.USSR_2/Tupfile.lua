if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("2.USSR_2.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "2.USSR_2.skn")
