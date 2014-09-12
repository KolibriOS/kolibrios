if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("opendial.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "opendial")
