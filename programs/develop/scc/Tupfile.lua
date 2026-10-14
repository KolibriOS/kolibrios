if tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.rule("SCC.ASM", FASM .. " %f %o " .. tup.getconfig("KPACK_CMD"), "SCC")
