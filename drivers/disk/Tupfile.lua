if tup.getconfig("NO_FASM") ~= "" then return end
ROOT = "../.."
tup.include(ROOT .. "/programs/use_fasm.lua")
tup.rule("tmpdisk.asm", "fasm %f %o " .. PESTRIP_CMD .. tup.getconfig("KPACK_CMD"), "%B.sys")
tup.rule("virt_disk.asm", "fasm %f %o " .. PESTRIP_CMD .. tup.getconfig("KPACK_CMD"), "%B.sys")