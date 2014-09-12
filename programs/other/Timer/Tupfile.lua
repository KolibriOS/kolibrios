if tup.getconfig("NO_NASM") ~= "" then return end
tup.rule("Timer.asm", "nasm -f bin -o %o %f " .. tup.getconfig("KPACK_CMD"), "timer")
