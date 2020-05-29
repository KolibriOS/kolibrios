if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("col.dtp.asm", 'fasm "%f" "%o"', "col.dtp")
tup.rule({"florence.asm", extra_inputs = {"col.dtp"}}, 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Florence.skn")
