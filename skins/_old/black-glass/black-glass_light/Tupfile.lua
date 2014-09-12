if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("black-glass_light.ASM", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "black-glass_light.skn")
