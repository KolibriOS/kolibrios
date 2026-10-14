if tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.rule("keymap.asm", FASM .. " %f %o", "piano.map")
tup.rule("piano.asm", FASM .. " -dlang=" .. tup.getconfig("LANG") .. " %f %o" .. tup.getconfig("KPACK_CMD"), "%B")
