if tup.getconfig("NO_TCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../programs"
end
tup.include(HELPERDIR .. "/use_tcc.lua")

LIBS = " -lbox_lib"

link_tcc({"passwordgen.c"}, "passwordgen"); 
