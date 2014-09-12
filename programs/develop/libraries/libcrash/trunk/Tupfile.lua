if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("libcrash.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "libcrash.obj")
tup.rule("crashtest.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "crashtest")
