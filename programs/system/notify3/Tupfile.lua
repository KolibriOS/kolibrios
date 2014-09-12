if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("notify.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "notify")
