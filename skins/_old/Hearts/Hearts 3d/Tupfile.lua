if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("hearts_3d.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Hearts 3d.skn")
