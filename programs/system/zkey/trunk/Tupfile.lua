if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("ZKEY.ASM", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "ZKEY")
