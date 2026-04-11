if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("log_el.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "log_el")
