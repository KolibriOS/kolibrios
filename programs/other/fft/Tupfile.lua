if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("FHT4A.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "FHT4A")
