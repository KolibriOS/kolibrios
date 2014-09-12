if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("3dspiral.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "3dspiral")
