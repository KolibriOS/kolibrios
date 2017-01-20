if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("launch.asm", "fasm %f %o -dlang=" .. tup.getconfig("LANG") .. tup.getconfig("KPACK_CMD"), "launch")
