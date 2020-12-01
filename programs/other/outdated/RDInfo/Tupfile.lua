if tup.getconfig("NO_NASM") ~= "" then return end
tup.rule("RDInfo.asm", "nasm -f bin -o %o %f " .. tup.getconfig("KPACK_CMD"), "RDInfo")
