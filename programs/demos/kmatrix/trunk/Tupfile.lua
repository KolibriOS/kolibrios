if tup.getconfig("NO_TCC") ~= "" then return end

HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_tcc.lua")

link_tcc("main.c", "kmatrix");
