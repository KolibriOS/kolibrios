SOUND_CWD = tup.getcwd() .. "/develop/sdk/trunk/sound"
SOUND_VAR = tup.getvariantdir() .. "/develop/sdk/trunk/sound"

INCLUDES = INCLUDES .. " -I" .. SOUND_CWD .. "/include"
table.insert(LIBDEPS, SOUND_VAR .. "/src/sound.lib")
LIBS = SOUND_VAR .. "/src/sound.lib " .. LIBS
