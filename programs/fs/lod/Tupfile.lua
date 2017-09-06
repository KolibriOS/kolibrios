if tup.getconfig("NO_NASM") ~= "" then return end
tup.rule("lod.asm", "nasm -f bin -o %o %f " .. tup.getconfig("KPACK_CMD"), "lod")
