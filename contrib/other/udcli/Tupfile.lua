if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

LDFLAGS = LDFLAGS .. " -Llibudis86"
LIBS = "-ludis86 " .. LIBS
table.insert(LIBDEPS, "libudis86/<libudis86>")
INCLUDES = INCLUDES .. " -Ilibudis86"
CFLAGS = CFLAGS .. " -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32"

compile_gcc{"udcli.c"}
link_gcc("udcli")
 
