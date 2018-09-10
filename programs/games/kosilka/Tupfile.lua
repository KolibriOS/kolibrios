if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_MSVC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_msvc.lua")
if tup.getconfig("LANG") == "ru"
then CFLAGS = CFLAGS .. " /DLANG_RUS"
else CFLAGS = CFLAGS .. " /DLANG_ENG"
end
compile_msvc{"kosilka.cpp", "KosFile.cpp", "kosSyst.cpp", "mcsmemm.cpp"}
link_msvc("kosilka")
