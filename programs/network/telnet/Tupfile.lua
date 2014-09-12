if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("telnet.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "telnet")
