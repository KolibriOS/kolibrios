if tup.getconfig("NO_TCC") ~= "" then return end

HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_tcc.lua")

LIBS = "-lbox_lib"

link_tcc({"main.c", "func.c", "parser.c"}, "graph");
