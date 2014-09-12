if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("STARTMUS.ASM", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "STARTMUS")
