if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("VIEW3DS.ASM", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "VIEW3DS")
