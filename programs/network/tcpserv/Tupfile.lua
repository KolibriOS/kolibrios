if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("tcpserv.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "tcpserv")
