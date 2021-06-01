if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("kruler.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "kruler")

 
