if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("view3ds.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "view3ds")
