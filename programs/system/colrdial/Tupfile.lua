if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("color_dialog.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "color_dialog")
