if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("ABOUT.TXT", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "ABOUT.TXT.KPACK")


