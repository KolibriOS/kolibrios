if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("fontconv.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "fontconv")
