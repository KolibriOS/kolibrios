if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("3dsheart.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "3dsheart")
