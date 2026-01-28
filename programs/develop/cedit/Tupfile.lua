if tup.getconfig("NO_OB07") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../"
end

tup.include(HELPERDIR .. "/use_ob07.lua")

OB07_FLAGS = OB07_FLAGS .. "-nochk a "

build_ob07({"SRC/CEdit.ob07"}, "cedit"); 
