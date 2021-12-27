if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("ABOUT.TXT", "kpack %f %o ", "ABOUT.TXT.KPACK")

