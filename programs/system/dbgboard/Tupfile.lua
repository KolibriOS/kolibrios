if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("dbgboard.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "dbgboard")
