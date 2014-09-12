if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("vncc.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "vncc")
