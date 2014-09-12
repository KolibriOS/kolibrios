if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("http.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "http.obj")
