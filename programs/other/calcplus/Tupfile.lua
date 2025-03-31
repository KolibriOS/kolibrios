if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("pipet.asm", FASM .. " -dlang=" .. tup.getconfig("LANG") .. " %f %o" .. tup.getconfig("KPACK_CMD"), "%B")
