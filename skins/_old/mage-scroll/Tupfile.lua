if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("colour.asm", 'fasm "%f" "%o"', "colour.dtp")
tup.rule({"default.asm", extra_inputs = {"colour.dtp"}}, 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "mage-scroll.skn")
