if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

if tup.getconfig("LANG") == "ru"
then C_LANG = "LANG_RUS"
else C_LANG = "LANG_ENG" -- this includes default case without config
end
LIBS = "-lsupc++ -lstdc++ " .. LIBS
LDFLAGS = LDFLAGS .. " --subsystem native"
CFLAGS = CFLAGS .. " -D_KOLIBRI -DAUTOBUILD -D" .. C_LANG .. " -DNO_FILES -fwhole-program"

compile_gcc{"checkers.cpp"}
link_gcc("checkers")
