if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("lod.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "lod")
