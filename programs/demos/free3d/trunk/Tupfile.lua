if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("free3d.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "free3d")
