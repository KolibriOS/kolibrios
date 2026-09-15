if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("kpack.asm", "fasm %f %o " .. tup.getconfig("KPACK_CMD"), "kpack")
-- Native host ports (windowless in-place compressor):
tup.rule("kpackw.asm", "fasm %f %o", "kpackw.exe") -- Windows PE
tup.rule("kpackl.asm", "fasm %f %o", "kpackl")     -- Linux ELF32
