#include "SDL_audio.h"
#include <menuet/os.h>
#include <stdlib.h>
#include <string.h>
#include <sound.h>
#include <stdio.h>

static void GetNotify(__u32* event)
{
        __asm__("int $0x40" :: "a"(68),"b"(14),"c"(event));
}
static int CreateThread(void* fn, char* p_stack)
{
        int res;
        __asm__("int $0x40" : "=a"(res) : "a"(51),"b"(1),"c"(fn),"d"(p_stack));
        return res;
}
static char pinfo[1024];
static int GetProcessInfo(int slot)
{
	int res;
	__asm__("int $0x40" : "=a"(res) : "a"(9),"b"(pinfo),"c"(slot));
	return res;
}
static void ActivateWnd(int slot)
{
	__asm__("int $0x40" :: "a"(18),"b"(3),"c"(slot));
}
static void Yield(void)
{
	__asm__("int $0x40" :: "a"(68),"b"(1));
}

static int bInitialized=0;
static SNDBUF hBuff=0;
static char* data=NULL;
static int audio_tid=0;
static int main_slot;
static __u32 main_tid;
static char audio_thread_stack[40960];
static __u32 used_format=0;
static volatile int mix_size=0;

static void (*callback)(void* userdata, Uint8* stream, int len);
static void* userdata;

int SDL_AudioInit(const char* driver_name)
{
	if (bInitialized)
	{
		SDL_SetError("audio already initialized");
		return -1;
	}
	int ver;
	if (InitSound(&ver))
	{
        	SDL_printf("Warning: cannot load drivers, sound output will be disabled\n");
        	return 0;
        }
        bInitialized = 1;
	return 0;
}

void SDL_AudioQuit(void)
{
}

char* SDL_AudioDriverName(char* namebuf, int maxlen)
{
        if (!bInitialized)
                return NULL;
        strncpy(namebuf,"KolibriAudio",maxlen);
        return namebuf;
}

#define AUDIO_SUSPEND 1
#define AUDIO_RESUME 2
#define AUDIO_DIE 3
static volatile int audio_command=0,audio_response=0,bLocked=0,bInCallback=0;
static void audio_thread(void)
{
	SDL_printf("audio_thread created\n");
        int bPaused;
        __u32 event[6];
        // initialize
        if (CreateBuffer(used_format|PCM_RING, 0, &hBuff))
        {
                audio_response=1;
                __menuet__sys_exit();
        }
        GetBufferSize(hBuff, &mix_size);
        SDL_printf("buffer created, size is %d\n",mix_size);
        mix_size >>= 1;
        data = malloc(mix_size);
        audio_response=1;
        if (!data) __menuet__sys_exit();
        // wait for resume
        while (audio_command!=AUDIO_RESUME)
                Yield();
        // initialize
/*        bInCallback=1;
        callback(userdata,data,mix_size);
        SetBuffer(hBuff,data,0,mix_size);
        callback(userdata,data,mix_size);
        SetBuffer(hBuff,data,mix_size,mix_size);
        bInCallback=0;*/
        audio_command=0;
        bPaused=0;
        audio_response=1;
        PlayBuffer(hBuff,0);
        // main loop
        for (;;)
        {
                if (audio_command==AUDIO_RESUME)
                {
                	PlayBuffer(hBuff,0);
                        audio_command = 0;
                        bPaused = 0;
                        audio_response = 1;
                }
                else if (audio_command==AUDIO_SUSPEND)
                {
                	StopBuffer(hBuff);
                        audio_command = 0;
                        bPaused = 1;
                        audio_response = 1;
                }
                else if (audio_command==AUDIO_DIE)
                {
                        audio_response = 1;
                        StopBuffer(hBuff);
                        DestroyBuffer(hBuff);
                        __menuet__sys_exit();
                }
                else
                {
                	GetProcessInfo(main_slot);
                	if (pinfo[0x32]==9 || *(__u32*)(pinfo+0x1E)!=main_tid)
                	{
                		audio_command = AUDIO_DIE;
                		continue;
                	}
                }
                if (bPaused)
                        __menuet__delay100(5);
                else
                {
                        GetNotify(event);
                        if (event[0] != 0xFF000001)
                        	continue;
                        while (bLocked)
                                Yield();
                        bInCallback=1;
                        callback(userdata,data,mix_size);
                        bInCallback=0;
                        SetBuffer(hBuff,data,event[3],mix_size);
                }
        }
}

int SDL_OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained)
{
        if (!bInitialized)
        {
                SDL_SetError("Audio device was not initialized");
                return -1;
        }
        if (!obtained)
        {
                SDL_SetError("Audio format: software emulation is not supported");
                return -1;
        }
        if (used_format)
        {
                SDL_SetError("Audio device was already opened");
                return -1;
        }
        memcpy(obtained,desired,sizeof(SDL_AudioSpec));
        switch (desired->freq)
        {

#define HANDLE_FREQ(freq,symb)                          \
        case freq:                                      \
                switch (desired->channels)              \
                {                                       \
                case 1:                                 \
                        switch (desired->format)        \
                        {                               \
                        case AUDIO_U8:                  \
                        case AUDIO_S8:                  \
                                used_format = PCM_1_8_##symb;   \
                                break;                  \
                        case AUDIO_U16SYS:              \
                        case AUDIO_S16SYS:              \
                                used_format = PCM_1_16_##symb;  \
                                break;                  \
                        }                               \
                        break;                          \
                case 2:                                 \
                        switch (desired->format)        \
                        {                               \
                        case AUDIO_U8:                  \
                        case AUDIO_S8:                  \
                                used_format = PCM_2_8_##symb;   \
                                break;                  \
                        case AUDIO_U16SYS:              \
                        case AUDIO_S16SYS:              \
                                used_format = PCM_2_16_##symb;  \
                                break;                  \
                        }                               \
                        break;                          \
                }                                       \
                break;

        HANDLE_FREQ(48000,48);
        HANDLE_FREQ(44100,44);
        HANDLE_FREQ(32000,32);
        HANDLE_FREQ(24000,24);
        HANDLE_FREQ(22050,22);
        HANDLE_FREQ(16000,16);
        HANDLE_FREQ(12000,12);
        HANDLE_FREQ(11025,11);
        HANDLE_FREQ(8000,8);
        }
        if (!used_format)
        {
                SDL_SetError("Unknown audio format");
                return -1;
        }
        callback=desired->callback;
        userdata=desired->userdata;
        GetProcessInfo(-1);
        main_tid = *(__u32*)(pinfo+0x1E);
        for (main_slot=0;;main_slot++)
        {
                GetProcessInfo(main_slot);
                if (pinfo[0x32]!=9 && *(__u32*)(pinfo+0x1E)==main_tid)
        	        break;
        }
        audio_tid=CreateThread(audio_thread,audio_thread_stack+40960);
        if (audio_tid<0)
        {
                SDL_SetError("Cannot create audio thread");
                return -1;
        }
        ActivateWnd(main_slot);
        while (!audio_response)
                Yield();
        if (!hBuff)
        {
                SDL_SetError("Cannot create audio buffer");
                return -1;
        }
        if (!data)
        {
                SDL_SetError("Cannot allocate audio buffer");
                return -1;
        }
        obtained->silence = (desired->format == AUDIO_U8 ? 0x80 : 0);
        obtained->size = mix_size;
        obtained->samples = obtained->size / obtained->channels;
        if (desired->format == AUDIO_U16SYS || desired->format == AUDIO_S16SYS)
                obtained->samples /= 2;
        SDL_printf("obtained size is %d, samples %d\n",obtained->size,
        obtained->samples);
        return 0;
}
void SDL_CloseAudio(void)
{
        if (!audio_tid) return;
        audio_response = 0;
        audio_command = AUDIO_DIE;
        while (!audio_response)
                Yield();
        free(data);
        used_format = 0;
}

void SDL_PauseAudio(int pause_on)
{
	if (!audio_tid) return;
        audio_response = 0;
        audio_command = pause_on?AUDIO_SUSPEND:AUDIO_RESUME;
        while (!audio_response)
                Yield();
}
void SDL_LockAudio(void)
{
	if (!audio_tid) return;
        bLocked = 1;
        while (bInCallback)
                Yield();
}
void SDL_UnlockAudio(void)
{
        bLocked = 0;
}
