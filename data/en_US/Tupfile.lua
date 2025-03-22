if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("welcome.htm", "cp %f %o" .. tup.getconfig("KPACK_CMD"), "welcome.htm.kpack")
