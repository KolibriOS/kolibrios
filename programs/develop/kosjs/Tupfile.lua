if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../.." or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

LDFLAGS = LDFLAGS .. " -Llibmujs --subsystem native"
LIBS = "-lmujs " .. LIBS
table.insert(LIBDEPS, "libmujs/<libmujs>")
INCLUDES = INCLUDES .. " -Ilibmujs"
 
compile_gcc{"kosjs.c", "import.c"}
link_gcc("kosjs")
 
