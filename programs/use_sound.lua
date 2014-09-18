SOUND = tup.getcwd() .. "/develop/sdk/trunk/sound"

INCLUDES = INCLUDES .. " -I" .. SOUND .. "/include"
table.insert(LIBDEPS, SOUND .. "/src/sound.lib")
LIBS = SOUND .. "/src/sound.lib " .. LIBS
