if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("pppoe.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "pppoe")
