if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("RasterWorks.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "RasterWorks.obj")
