if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("firework.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "firework")
