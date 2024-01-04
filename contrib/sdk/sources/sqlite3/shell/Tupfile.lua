if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../../../../../programs"
end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

INCLUDES= INCLUDES .. " -I .. "

CFLAGS = CFLAGS .. " -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -DSQLITE_OS_OTHER=1 -DHAVE_UNISTD_H=0 -D_NO_STDERR -DSQLITE_OMIT_POPEN -DSQLITE_THREADSAFE=0 -D_KOLIBRI -DSQLITE_OMIT_VIRTUALTABLE -U__linux__ -DPACKAGE_NAME=\"sqlite\" -DPACKAGE_TARNAME=\"sqlite\" -DPACKAGE_VERSION=\"3.36.0\" -DPACKAGE_STRING=\"sqlite-3.36.0\" -DPACKAGE=\"sqlite\" -DVERSION=\"3.36.0\""

LDFLAGS = LDFLAGS .. " --subsystem console "

table.insert(LIBDEPS,"../../../lib/<libsqlite3.dll.a>")
LIBS = LIBS .. " -lsqlite3.dll" 

-- Compile --
compile_gcc{ 
    "shell.c", "stub.c"
}

-- Link --
link_gcc("sqlite3")


 
