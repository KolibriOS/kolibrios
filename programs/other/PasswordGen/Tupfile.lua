if tup.getconfig("NO_TCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_tcc.lua")

LIBS = " -lbox_lib"

link_tcc({"passwordgen.c"}, "passwordgen"); 
