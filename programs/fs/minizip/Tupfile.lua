if tup.getconfig("NO_GCC") ~= "" then return end
if tup.getconfig("HELPERDIR") == ""
then
  HELPERDIR = "../.."
end
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

CFLAGS = CFLAGS .. " -U__WIN32__ -U_Win32 -U_WIN32 -U__MINGW32__ -UWIN32 -Dunix -I. -I../../../contrib/sdk/sources/zlib"

LDFLAGS = LDFLAGS .. " --subsystem console -L../../../contrib/sdk/sources/zlib"

table.insert(LIBDEPS,"../../../contrib/sdk/lib/<libz.dll.a>")
LIBS = LIBS .. " -lz.dll" 

-- Compile --
compile_gcc{ 
    "miniunz.c", "unzip.c", "ioapi.c", "minizip.c", "zip.c",
}

-- Link miniunz
link_gcc({"miniunz.o", "unzip.o", "ioapi.o"},"miniunz")

-- Link minizip
link_gcc({"minizip.o", "zip.o", "ioapi.o"}, "minizip")



