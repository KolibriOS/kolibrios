if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("tftpc.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "tftpc")
