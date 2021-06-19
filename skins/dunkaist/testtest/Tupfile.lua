if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("dtp.asm", 'fasm "%f" "%o"', "%B")
tup.rule({"testtest.asm", extra_inputs = {"dtp"}}, 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "testtest.skn")
