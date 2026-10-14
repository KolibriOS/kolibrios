if tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.rule("mstate.asm", FASM .. " -dlang=" .. tup.getconfig("LANG") .. " %f %o " .. tup.getconfig("KPACK_CMD"), "mstate")
