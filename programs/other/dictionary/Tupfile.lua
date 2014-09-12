if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("dictionary.ASM", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "dictionary")
