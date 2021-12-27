if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("dtp.asm", 'fasm "%f" "%o"', "dtp.dtp")
tup.rule({"default.asm", extra_inputs = {"dtp.dtp"}}, 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "loggy.skn")
