if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.foreach_rule("*.asm", FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "%B")
