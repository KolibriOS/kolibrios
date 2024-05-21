TINYGL_CWD = tup.getcwd() .. "/develop/libraries/TinyGL"
TINYGL_VAR = tup.getvariantdir() .. "/develop/libraries/TinyGL"

INCLUDES = INCLUDES .. " -I" .. TINYGL_CWD .. "/include"
table.insert(LIBDEPS, TINYGL_VAR .. "/<>")
LIBS = TINYGL_VAR .. "/lib/libTinyGL.a " .. LIBS
