if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("hearts_flat.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Hearts flat.skn")
