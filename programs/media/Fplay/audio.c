
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <stdio.h>
#include <string.h>
#include "sound.h"
#include "fplay.h"


astream_t astream;

static SNDBUF hBuff;

extern volatile uint32_t status;

void audio_thread(void *param);

void spinlock_lock(volatile uint32_t *val)
{
    uint32_t tmp;

    __asm__ __volatile__ (
"0:\n\t"
    "mov %0, %1\n\t"
    "testl %1, %1\n\t"
    "jz 1f\n\t"

    "movl $68, %%eax\n\t"
    "movl $1,  %%ebx\n\t"
    "int  $0x40\n\t"
    "jmp 0b\n\t"
"1:\n\t"
    "incl %1\n\t"
    "xchgl %0, %1\n\t"
    "testl %1, %1\n\t"
	"jnz 0b\n"
    : "+m" (*val), "=&r"(tmp)
    ::"eax","ebx" );
}

static int snd_format;
int sample_rate;

int init_audio(int format)
{
    int    err;
    int    version =-1;
    char  *errstr;

    if((err = InitSound(&version)) !=0 )
    {
        errstr = "Sound service not installed\n\r";
        goto exit_whith_error;
    }
    printf("sound version 0x%x\n", version);

    if( (SOUND_VERSION>(version&0xFFFF)) ||
        (SOUND_VERSION<(version >> 16)))
    {
        errstr = "Sound service version mismatch\n\r";
        goto exit_whith_error;
    }

    snd_format = format;

    asm volatile ( "xchgw %bx, %bx");

    create_thread(audio_thread, 0, 163840);

    return 1;

exit_whith_error:

    printf(errstr);
    return 0;
};

static uint64_t samples_lost;
static double  audio_delta;

double get_master_clock()
{
    double tstamp;

    GetTimeStamp(hBuff, &tstamp);
    return tstamp - audio_delta;
};


void audio_thread(void *param)
{
    SND_EVENT evnt;
    int       buffsize;
    int      samples;
    int       err;
    char     *errstr;


    if((err = CreateBuffer(snd_format|PCM_RING,0, &hBuff)) != 0)
    {
        errstr = "Cannot create sound buffer\n\r";
        goto exit_whith_error;
    };

    SetVolume(hBuff,-1000,-1000);

    if((err = GetBufferSize(hBuff, &buffsize)) != 0)
    {
        errstr = "Cannot get buffer size\n\r";
        goto exit_whith_error;
    };

    buffsize = buffsize/2;

    samples = buffsize/4;

    while( (astream.count < buffsize*2) &&
               (status != 0) )
        yield();

    spinlock_lock(&astream.lock);
    {
        SetBuffer(hBuff, astream.buffer, 0, buffsize);
        astream.count -= buffsize;
        if(astream.count)
            memcpy(astream.buffer, astream.buffer+buffsize, astream.count);
        spinlock_unlock(&astream.lock);
    };

    if((err = PlayBuffer(hBuff, 0)) !=0 )
    {
        errstr = "Cannot play buffer\n\r";
        goto exit_whith_error;
    };


#ifdef BLACK_MAGIC_SOUND

    while( status != 0)
    {
        uint32_t  offset;

        GetNotify(&evnt);

        if(evnt.code != 0xFF000001)
        {
            printf("invalid event code %d\n\r", evnt.code);
            continue;
        }

        if(evnt.stream != hBuff)
        {
            printf("invalid stream %x hBuff= %x\n\r",
                    evnt.stream, hBuff);
            continue;
        }

        GetTimeStamp(hBuff, &audio_delta);
        samples_lost = audio_delta*sample_rate/1000;

        offset = evnt.offset;

        spinlock_lock(&astream.lock);
        {
            SetBuffer(hBuff, astream.buffer, offset, buffsize);
            astream.count -= buffsize;
            if(astream.count)
                memcpy(astream.buffer, astream.buffer+buffsize, astream.count);
            spinlock_unlock(&astream.lock);
        };
        break;
    };
#endif

    printf("initial audio delta %f\n", audio_delta);

    while( status != 0)
    {
        uint32_t  offset;
        double    event_stamp, wait_stamp;
        int       too_late = 0;

        GetNotify(&evnt);

        if(evnt.code != 0xFF000001)
        {
            printf("invalid event code %d\n\r", evnt.code);
            continue;
        }

        if(evnt.stream != hBuff)
        {
            printf("invalid stream %x hBuff= %x\n\r",
                    evnt.stream, hBuff);
            continue;
        };

        GetTimeStamp(hBuff, &event_stamp);

        offset = evnt.offset;

        while( (astream.count < buffsize) &&
               (status != 0) )
        {
            yield();
            GetTimeStamp(hBuff, &wait_stamp);
            if( (wait_stamp - event_stamp) >
                 samples*1500/sample_rate )
            {
                samples_lost+= samples;
                audio_delta = (double)samples_lost*1000/sample_rate;
//                printf("audio delta %f\n", audio_delta);
                too_late = 1;
                break;
            }
        };

        if((too_late == 1) || (status == 0))
            continue;

        spinlock_lock(&astream.lock);
        SetBuffer(hBuff, astream.buffer, offset, buffsize);
        astream.count -= buffsize;
        if(astream.count)
            memcpy(astream.buffer, astream.buffer+buffsize, astream.count);
        spinlock_unlock(&astream.lock);
    }

    return;

exit_whith_error:

    printf(errstr);
    return ;

};

