if tup.getconfig("NO_OB07") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../"
end

tup.include(HELPERDIR .. "/use_ob07.lua")

build_ob07({"SRC/FB2READ.ob07"}, "fb2read", "-upper");
