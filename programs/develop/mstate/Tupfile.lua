if tup.getconfig("NO_NASM") ~= "" then return end
tup.rule("mstate.asm", "nasm -t -f bin -o %o %f " .. tup.getconfig("KPACK_CMD"), "mstate")
