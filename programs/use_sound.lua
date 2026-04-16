SOUND_CWD = tup.getcwd() .. "/develop/sdk/sound"
SOUND_VAR = tup.getvariantdir() .. "/develop/sdk/sound"

INCLUDES = INCLUDES .. " -I" .. SOUND_CWD .. "/include"
table.insert(LIBDEPS, SOUND_VAR .. "/src/sound.lib")
LIBS = SOUND_VAR .. "/src/sound.lib " .. LIBS
