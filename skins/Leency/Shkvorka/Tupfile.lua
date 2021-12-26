if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("colors.dtp.asm", 'fasm "%f" "%o"', "colors.dtp")
tup.rule({"default.asm", extra_inputs = {"colors.dtp"}}, 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Shkvorka.skn")

