if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("fillscr.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "fillscr")
