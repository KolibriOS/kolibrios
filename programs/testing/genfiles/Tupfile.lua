if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("GenFiles.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "GenFiles")
