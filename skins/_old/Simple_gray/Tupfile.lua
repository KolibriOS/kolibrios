if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("simple_gray.ASM", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Simple_gray.skn")
