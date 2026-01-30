if tup.getconfig("NO_OB07") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../"
end

tup.include(HELPERDIR .. "/use_ob07.lua")

build_ob07({"SRC/CEdit.ob07"}, "cedit"); 
