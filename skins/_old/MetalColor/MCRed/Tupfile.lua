if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("MCRed.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "MCRed.skn")
