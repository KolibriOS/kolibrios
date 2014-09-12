if tup.getconfig("NO_MSVC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_msvc.lua")
compile_msvc{"*.cpp"}
link_msvc("table")
