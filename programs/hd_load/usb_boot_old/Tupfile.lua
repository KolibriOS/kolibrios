if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("MeOSload.asm", "fasm %f %o", "MeOSload.com")
tup.rule("enable.asm", "fasm %f %o", "enable.exe")
