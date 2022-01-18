if tup.getconfig("NO_TCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_tcc.lua")

LIBS = "-lhttp -limg"

link_tcc({"weather.c", "json/json.c"}, "weather"); 
