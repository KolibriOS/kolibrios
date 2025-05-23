if tup.getconfig("NO_FASM") ~= "" then return end
ROOT = "../../.."
tup.rule("usbother.asm", "fasm %f %o " .. tup.getconfig("PESTRIP_CMD") .. tup.getconfig("KPACK_CMD"), "%B.sys")
tup.rule("usbdrv.asm", "fasm %f %o ", "%B.dat")
