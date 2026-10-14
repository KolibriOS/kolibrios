if tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.rule("SOKO.ASM", FASM .. " -dlang=" .. tup.getconfig("LANG") .. " %f %o " .. tup.getconfig("KPACK_CMD"), "SOKO")
