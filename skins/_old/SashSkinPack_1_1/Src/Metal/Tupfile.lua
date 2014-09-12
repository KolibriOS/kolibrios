if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Metal.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Metal.skn")
