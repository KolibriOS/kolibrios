if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("planet_v.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "planet_v")
