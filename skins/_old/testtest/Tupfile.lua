if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("testtest.dtp.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "testtest.dtp")
tup.rule({"testtest.asm", extra_inputs = {"testtest.dtp"}}, 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "testtest.skn")
