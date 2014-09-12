if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Victory_1945.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Victory_1945.skn")
