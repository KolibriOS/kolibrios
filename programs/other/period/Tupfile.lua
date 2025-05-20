if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("period.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "period")
