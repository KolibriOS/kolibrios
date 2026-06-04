if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("httpget.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "httpget")
