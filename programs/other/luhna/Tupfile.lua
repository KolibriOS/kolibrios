if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("luhna.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "luhna")
