if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("chip8.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "chip8")

 