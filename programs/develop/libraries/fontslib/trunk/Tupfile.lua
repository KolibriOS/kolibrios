if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("font_.asm", "fasm %f %o", "font01.ksf")
tup.rule("fonts_lib.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "fonts_lib.obj")
