if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("nu_pogod.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "nu_pogod")
