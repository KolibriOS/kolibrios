if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

if tup.getconfig("LANG") == "ru"
then CFLAGS = CFLAGS .. " -Dlang_ru"
else CFLAGS = CFLAGS .. " -Dlang_en"
end

compile_gcc{"Backy.c"}
link_gcc("Backy")
