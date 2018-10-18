if tup.getconfig("NO_FASM") ~= "" or tup.getconfig("NO_MSVC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_msvc.lua")
compile_msvc{"rforces.cpp", "kosFile.cpp", "kosSyst.cpp", "mcsmemm.cpp"}
link_msvc("rforces")
