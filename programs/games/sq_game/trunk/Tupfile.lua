if tup.getconfig("NO_FASM") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_fasm.lua")
tup.rule("SQ_GAME.ASM", FASM .. " -dlang=" .. tup.getconfig("LANG") .. " %f %o " .. tup.getconfig("KPACK_CMD"), "SQ_GAME")
