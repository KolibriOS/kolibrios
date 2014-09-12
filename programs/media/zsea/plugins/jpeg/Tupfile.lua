if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("cnv_jpeg.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "cnv_jpeg.obj")
