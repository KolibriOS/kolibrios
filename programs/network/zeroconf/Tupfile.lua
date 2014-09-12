if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("zeroconf.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "zeroconf")
