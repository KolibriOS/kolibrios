if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("hunter.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "hunter")
