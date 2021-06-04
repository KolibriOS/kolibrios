if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("entropyview.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "entropyview")

 
