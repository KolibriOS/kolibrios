if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("mp3info.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "mp3info")
