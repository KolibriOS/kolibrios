if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Dark_future.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Dark_future.skn")
