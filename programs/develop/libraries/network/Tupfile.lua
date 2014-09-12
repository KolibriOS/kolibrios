if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("network.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "network.obj")
