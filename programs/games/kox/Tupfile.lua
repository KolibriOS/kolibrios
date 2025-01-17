if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("kox.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "kox")
