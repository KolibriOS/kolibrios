if tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.rule("charsets.asm", FASM .. " -dlang=" .. tup.getconfig("LANG") .. " %f %o" .. tup.getconfig("KPACK_CMD"), "%B")
