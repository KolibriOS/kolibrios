if tup.getconfig("NO_TCC") ~= "" or tup.getconfig("NO_FASM") ~= "" then return end
tup.include(HELPERDIR .. "/use_tcc.lua")

LIBS = "-lhttp -limg"

link_tcc({"weather.c", "json/json.c"}, "weather"); 
