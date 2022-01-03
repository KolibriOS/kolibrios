#include "audio.h"
#include "../options.h"
#include <SDL.h>

int music_volume = 4;

void PHL_AudioInit()
{
    SDL_InitSubSystem(SDL_INIT_AUDIO);
    #ifndef __MORPHOS__
    Mix_Init(MIX_INIT_OGG); // midi is on by default
    #endif
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    PHL_MusicVolume(0.25f * music_volume);
}

void PHL_AudioClose()
{
    Mix_CloseAudio();
    #ifndef __MORPHOS__
    Mix_Quit();
    #endif
}

//Same as PHL_LoadSound, but expects a file name without extension
PHL_Music PHL_LoadMusic(char* fname, int loop)
{
    PHL_Music ret;
    ret.loop = loop;
    char buff[4096];
    strcpy(buff, "data/");
    strcat(buff, fname);
    strcat(buff, getMusicType()?".ogg":".mid");
    ret.snd = Mix_LoadMUS(buff);
    return ret;
}

PHL_Sound PHL_LoadSound(char* fname)
{
    char buff[4096];
    strcpy(buff, "data/");
    strcat(buff, fname);
    return Mix_LoadWAV(buff);
}

void PHL_MusicVolume(float vol)
{
    Mix_VolumeMusic(SDL_MIX_MAXVOLUME*vol);
}

void PHL_PlayMusic(PHL_Music snd)
{
    if(snd.snd)
        Mix_PlayMusic(snd.snd, snd.loop?-1:0);
}

void PHL_PlaySound(PHL_Sound snd, int channel)
{
    Mix_PlayChannel(channel, snd, 0);
}

void PHL_StopMusic()
{
    Mix_HaltMusic();
}

void PHL_StopSound(PHL_Sound snd, int channel)
{
    Mix_HaltChannel(channel);
}

void PHL_FreeMusic(PHL_Music snd)
{
    if(snd.snd)
        Mix_FreeMusic(snd.snd);
    snd.snd = NULL;
}

void PHL_FreeSound(PHL_Sound snd)
{
    Mix_FreeChunk(snd);
}
