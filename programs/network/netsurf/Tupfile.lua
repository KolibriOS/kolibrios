if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("nsinstall.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "nsinstall")
