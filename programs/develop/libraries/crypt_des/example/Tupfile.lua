if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("crypt_files.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "crypt_files")
