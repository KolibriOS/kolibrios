if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("mos3de.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "mos3de")
