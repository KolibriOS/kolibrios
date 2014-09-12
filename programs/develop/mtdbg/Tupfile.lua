if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("mtdbg.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "mtdbg")
