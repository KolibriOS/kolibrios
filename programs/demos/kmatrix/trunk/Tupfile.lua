if tup.getconfig("NO_TCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../../programs"
end
tup.include(HELPERDIR .. "/use_tcc.lua")

link_tcc("main.c", "kmatrix");
