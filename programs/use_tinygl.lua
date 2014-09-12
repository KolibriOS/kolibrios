TINYGL = tup.getcwd() .. "/develop/libraries/TinyGL"

INCLUDES = INCLUDES .. " -I" .. TINYGL .. "/include"
table.insert(LIBDEPS, TINYGL .. "/<>")
LIBS = TINYGL .. "/lib/libTinyGL.a " .. LIBS
