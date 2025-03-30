if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("block.asm", "fasm -m 65636 %f %o " .. tup.getconfig("KPACK_CMD"), "block.bin")
