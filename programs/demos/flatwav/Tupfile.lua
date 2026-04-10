if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("FLATWAV.ASM", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "flatwav")
