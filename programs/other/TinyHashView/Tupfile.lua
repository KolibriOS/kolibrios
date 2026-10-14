if tup.getconfig("NO_TCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_tcc.lua")

LIBS = "-lcryptal -lbox_lib -ldialog"

link_tcc({"thashview.c"}, "thashview", {"cryptal"});
