if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("megamaze.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "megamaze")
