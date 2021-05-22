if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../.." or tup.getconfig("HELPERDIR")
ROOT = "../../../../.."
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.foreach_rule("*.asm", FASM .. " %f %o " .. PESTRIP_CMD .. tup.getconfig("KPACK_CMD"), "%B")
