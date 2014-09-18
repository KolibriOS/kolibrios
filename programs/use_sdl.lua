SDL_INCLUDE = tup.getcwd() .. "/../contrib/sdk/sources/SDL-1.2.2/include"
SDL_LIB = tup.getcwd() .. "/../contrib/sdk/lib"

tup.include("use_sound.lua")

INCLUDES = INCLUDES .. " -I" .. SDL_INCLUDE
table.insert(LIBDEPS, SDL_LIB .. "/<libSDL>")
LIBS = SDL_LIB .. "/libSDL.a " .. LIBS
