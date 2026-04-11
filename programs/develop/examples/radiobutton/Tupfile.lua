if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("optionbox.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "optionbox")
