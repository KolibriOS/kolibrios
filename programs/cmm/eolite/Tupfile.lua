if tup.getconfig("NO_CMM") ~= "" then return end
if tup.getconfig("NO_TCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_tcc.lua")

if tup.getconfig("LANG") == "ru"
then C_LANG = "LANG_RUS"
else C_LANG = "LANG_ENG" -- this includes default case without config
end

LFLAGS = LFLAGS .. " -stack=20480"
LIBS += " ../../develop/ktcc/trunk/bin/lib/tiny.o -nostdlib -lbox_lib -lini -limg"

tup.rule("Eolite.c", "c-- -coff /D=" .. C_LANG .. " /OPATH=%o %f", "%B.obj")
tup.rule("Eolite.obj", "objconv -felf32 %f %o", "%B.o")
link_tcc("Eolite.o", "%B.com")
