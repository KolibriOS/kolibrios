if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("editbox.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "editbox")
