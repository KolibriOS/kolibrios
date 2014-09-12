if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("mursky.dtp.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "mursky.dtp")
tup.rule({"mursky.asm", extra_inputs = {"mursky.dtp"}}, 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "mursky.skn")
