#include "SDL_audio.h"
#include <stdint.h>
#include <sys/ksys.h>
#include <stdlib.h>
#include <string.h>
#include <sound.h>
#include <stdio.h>

extern void SDL_printf(const char * fmt,...);

#define AUDIO_THREAD_STACK_SIZE 40960

static ksys_thread_t thread_info;
static int bInitialized = 0;
static SNDBUF hBuff = 0;
static uint8_t* data = NULL;
static int audio_tid = 0;
static int main_slot;
static uint32_t main_tid;
static char audio_thread_stack[AUDIO_THREAD_STACK_SIZE];
static uint32_t used_format = 0;
static int mix_size = 0;

static void (*callback)(void* userdata, Uint8* stream, int len);
static void* userdata;

int SDL_AudioInit(const char* driver_name)
{
    if (bInitialized) {
        SDL_SetError("Audio already initialized");
        return -1;
    }
    int ver;
    if (InitSound(&ver)) {
        SDL_printf("Warning: cannot load drivers, sound output will be disabled\n");
        return 0;
    }
    bInitialized = 1;
    return 0;
}

void SDL_AudioQuit(void) {/*STUB*/}

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

static volatile int audio_command=0, audio_response=0, bLocked=0, bInCallback=0;

static void audio_thread(void)
{
    SDL_printf("Audio_thread created\n");
    int bPaused;
    ksys_signal_info_t snd_signal;
        // initialize
    if (CreateBuffer(used_format|PCM_RING, 0, &hBuff)) {
        audio_response=1;
        exit(0);
    }
    
    GetBufferSize(hBuff, &mix_size);
    SDL_printf("buffer created, size is %d\n", mix_size);
    mix_size >>= 1;
    data = malloc(mix_size);
    audio_response = 1;
    if (!data) exit(0);
    
    // wait for resume
    while (audio_command != AUDIO_RESUME)
        _ksys_thread_yield();
    audio_command = 0;
    bPaused = 0;
    audio_response = 1;
    PlayBuffer(hBuff, 0);
    
    // main loop
    while(1) {
        if (audio_command == AUDIO_RESUME) {
            PlayBuffer(hBuff, 0);
            audio_command = 0;
            bPaused = 0;
            audio_response = 1;
        } else if (audio_command == AUDIO_SUSPEND) {
            StopBuffer(hBuff);
            audio_command = 0;
            bPaused = 1;
            audio_response = 1;
        }else if (audio_command == AUDIO_DIE) {
            audio_response = 1;
            StopBuffer(hBuff);
            DestroyBuffer(hBuff);
            exit(0);
        } else {
            _ksys_thread_info(&thread_info, main_slot);
            if (thread_info.slot_state == KSYS_SLOT_STATE_FREE || thread_info.pid !=main_tid) {
                audio_command = AUDIO_DIE;
                continue;
            }
        }
        if (bPaused) {
            _ksys_delay(5);
        } else {
            _ksys_wait_signal(&snd_signal);
            if (snd_signal.id != 0xFF000001)
                continue;
            while (bLocked)
                _ksys_thread_yield();

            bInCallback=1;
            callback(userdata, data, mix_size);
            bInCallback=0;
            SetBuffer(hBuff, data, ((int*)snd_signal.data)[2], mix_size);
        }
    }
}

int SDL_OpenAudio(SDL_AudioSpec* desired, SDL_AudioSpec* obtained)
{
    if (!bInitialized) {
        SDL_SetError("Audio device was not initialized");
        return -1;
    }
    
    if (!obtained) {
        SDL_SetError("Audio format: software emulation is not supported");
        return -1;
    }
        
    if (used_format) {
        SDL_SetError("Audio device was already opened");
        return -1;
    }
    memcpy(obtained, desired, sizeof(SDL_AudioSpec));
    switch (desired->freq) {

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

    if (!used_format) {
        SDL_SetError("Unknown audio format");
        return -1;
    }

    callback = desired->callback;
    userdata = desired->userdata;
        
    _ksys_thread_info(&thread_info, KSYS_THIS_SLOT);
    main_tid = thread_info.pid;
    for (main_slot=0 ;; main_slot++) {
        _ksys_thread_info(&thread_info, main_slot);
        if (thread_info.slot_state != KSYS_SLOT_STATE_FREE && thread_info.pid == main_tid)
            break;
    }
    audio_tid = _ksys_create_thread(audio_thread, audio_thread_stack+AUDIO_THREAD_STACK_SIZE);
    if (audio_tid < 0) {
        SDL_SetError("Cannot create audio thread");
            return -1;
    }
    
    _ksys_focus_window(main_slot);
    while (!audio_response)
        _ksys_thread_yield();
    
    if (!hBuff) {
        SDL_SetError("Cannot create audio buffer");
        return -1;
    }
    if (!data){
        SDL_SetError("Cannot allocate audio buffer");
        return -1;
    }
    obtained->silence = (desired->format == AUDIO_U8 ? 0x80 : 0);
    obtained->size = mix_size;
    obtained->samples = obtained->size / obtained->channels;
        
    if (desired->format == AUDIO_U16SYS || desired->format == AUDIO_S16SYS)
        obtained->samples /= 2;
    
    SDL_printf("obtained size is %d, samples %d\n", obtained->size, obtained->samples);
    return 0;
}
void SDL_CloseAudio(void)
{
    if (!audio_tid) return;
    audio_response = 0;
    audio_command = AUDIO_DIE;
    while (!audio_response) _ksys_thread_yield();
    free(data);
    used_format = 0;
}

void SDL_PauseAudio(int pause_on)
{
    if (!audio_tid) return;
    audio_response = 0;
    audio_command = pause_on ? AUDIO_SUSPEND : AUDIO_RESUME;
    while (!audio_response) _ksys_thread_yield();
}
void SDL_LockAudio(void)
{
    if (!audio_tid) return;
    bLocked = 1;
    while (bInCallback) _ksys_thread_yield();
}
void SDL_UnlockAudio(void)
{
    bLocked = 0;
}
