if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule({"koldbg.asm"}, "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "koldbg")
