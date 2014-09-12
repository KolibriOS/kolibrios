if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("SCC.ASM", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "SCC")
