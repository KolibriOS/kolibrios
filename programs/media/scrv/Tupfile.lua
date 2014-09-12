if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("scrv.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "scrv")
