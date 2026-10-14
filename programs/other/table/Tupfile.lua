if tup.getconfig("NO_MSVC") ~= "" then return end
tup.include(HELPERDIR .. "/use_msvc.lua")
compile_msvc{"*.cpp"}
link_msvc("table")
