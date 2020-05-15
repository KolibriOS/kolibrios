if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Win10.dtp.asm", 'fasm "%f" "%o"', "Win10.dtp")
tup.rule({"Win10.asm", extra_inputs = {"Win10.dtp"}}, 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Win10.skn")
