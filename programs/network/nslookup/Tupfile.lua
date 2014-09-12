if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("nslookup.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "nslookup")
