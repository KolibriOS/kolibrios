if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("palitra.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "palitra")
