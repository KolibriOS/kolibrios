if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("zeroline.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "zeroline")