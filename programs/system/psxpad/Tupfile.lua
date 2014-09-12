if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("psxpad.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "psxpad")
