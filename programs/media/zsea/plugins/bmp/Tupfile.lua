if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("cnv_bmp.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "cnv_bmp.obj")
