if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("1.MetalGearArrows.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "MetalGearArrows.skn")
