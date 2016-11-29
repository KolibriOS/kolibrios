if tup.getconfig("NO_FASM") ~= "" then return end
ROOT="../../../.."
tup.rule("main.asm", "fasm %f %o " .. tup.getconfig("PESTRIP_CMD") .. tup.getconfig("KPACK_CMD"), "%B.exe")
tup.rule("forwarder.asm", "fasm %f %o " .. tup.getconfig("PESTRIP_CMD") .. tup.getconfig("KPACK_CMD"), "%B.dll")
tup.rule("forwarded.asm", "fasm %f %o " .. tup.getconfig("PESTRIP_CMD") .. tup.getconfig("KPACK_CMD"), "%B.dll")
