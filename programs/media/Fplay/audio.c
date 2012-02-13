
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

extern uint8_t *decoder_buffer;

extern volatile uint32_t status;

extern volatile uint32_t driver_lock;


static int snd_format;
int sample_rate;

int init_audio(int format)
{
    int    err;
    int    version =-1;
    char  *errstr;

    mutex_lock(&driver_lock);

    if((err = InitSound(&version)) !=0 )
    {
        mutex_unlock(&driver_lock);
        errstr = "Sound service not installed\n\r";
        goto exit_whith_error;
    };

    mutex_unlock(&driver_lock);

    printf("sound version 0x%x\n", version);

    if( (SOUND_VERSION>(version&0xFFFF)) ||
        (SOUND_VERSION<(version >> 16)))
    {
        errstr = "Sound service version mismatch\n\r";
        goto exit_whith_error;
    }

    snd_format = format;

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

int decode_audio(AVCodecContext  *ctx, queue_t *qa)
{
    AVPacket   pkt;
    AVPacket    pkt_tmp;

    uint8_t    *audio_data;
    int         audio_size;
    int         len;
    int         data_size=0;

    if( astream.count > AVCODEC_MAX_AUDIO_FRAME_SIZE*7)
        return 1;

    if( get_packet(qa, &pkt) == 0 )
        return 0;

 //          __asm__("int3");

    pkt_tmp = pkt;

    while(pkt_tmp.size > 0)
    {
        data_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;

        len = avcodec_decode_audio3(ctx,(int16_t*)decoder_buffer,
                                   &data_size, &pkt_tmp);

        if(len >= 0)
        {
            pkt_tmp.data += len;
            pkt_tmp.size -= len;

            mutex_lock(&astream.lock);
            memcpy(astream.buffer+astream.count, decoder_buffer, data_size);
            astream.count += data_size;
            mutex_unlock(&astream.lock);
       }
       else pkt_tmp.size = 0;
    }
    av_free_packet(&pkt);
    return 1;
};


int audio_thread(void *param)
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

    mutex_lock(&astream.lock);
    {
        SetBuffer(hBuff, astream.buffer, 0, buffsize);
        astream.count -= buffsize;
        if(astream.count)
            memcpy(astream.buffer, astream.buffer+buffsize, astream.count);
        mutex_unlock(&astream.lock);
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

        mutex_lock(&astream.lock);
        {
            SetBuffer(hBuff, astream.buffer, offset, buffsize);
            astream.count -= buffsize;
            if(astream.count)
                memcpy(astream.buffer, astream.buffer+buffsize, astream.count);
            mutex_unlock(&astream.lock);
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

        mutex_lock(&astream.lock);
        SetBuffer(hBuff, astream.buffer, offset, buffsize);
        astream.count -= buffsize;
        if(astream.count)
            memcpy(astream.buffer, astream.buffer+buffsize, astream.count);
        mutex_unlock(&astream.lock);
    }

    StopBuffer(hBuff);
    DestroyBuffer(hBuff);

    return 0;

exit_whith_error:

    printf(errstr);
    return -1;

};

