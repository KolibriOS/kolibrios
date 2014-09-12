if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Win311.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "WIN311.skn")
