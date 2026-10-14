if tup.getconfig("NO_TCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end

tup.include(HELPERDIR .. "/use_tcc.lua")

link_tcc("main.c", "kmatrix")
