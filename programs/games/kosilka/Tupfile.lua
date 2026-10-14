if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_MSVC") ~= "" then return end
tup.include(HELPERDIR .. "/use_msvc.lua")
if tup.getconfig("LANG") == "ru_RU"
then CFLAGS += " /DLANG=RUS"
else CFLAGS += " /DLANG=ENG"
end
compile_msvc{"kosilka.cpp", "KosFile.cpp", "kosSyst.cpp", "mcsmemm.cpp"}
link_msvc("kosilka")
