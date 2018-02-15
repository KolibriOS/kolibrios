if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("UNZ.ASM", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "unz")
