if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("PrMK.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "PrMK")
