if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("@numcalc.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "@numcalc")
