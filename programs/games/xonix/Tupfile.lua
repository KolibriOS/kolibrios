if tup.getconfig("NO_MSVC") ~= "" then return end
tup.include(HELPERDIR .. "/use_msvc.lua")
if tup.getconfig("LANG") == "ru_RU"
then CFLAGS = CFLAGS .. " /DLANG=RUS"
else CFLAGS = CFLAGS .. " /DLANG=ENG"
end
compile_msvc{"*.cpp"}
link_msvc("xonix")
