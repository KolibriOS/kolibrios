if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("rtfread.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "rtfread")
