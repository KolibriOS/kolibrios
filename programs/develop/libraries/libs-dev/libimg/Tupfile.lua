if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("libimg.asm", "fasm -m 32768 %f %o " .. tup.getconfig("KPACK_CMD"), "libimg.obj")
