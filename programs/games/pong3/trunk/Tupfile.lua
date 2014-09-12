if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("pong3.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "pong3")
