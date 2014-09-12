if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("4.MetalGearArrows.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "4.MetalGearArrows.skn")
