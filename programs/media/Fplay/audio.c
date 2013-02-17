
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#include <stdio.h>
#include <string.h>
#include "../winlib/winlib.h"
#include "sound.h"
#include "fplay.h"


astream_t astream;

extern uint8_t *decoder_buffer;
int resampler_size;
volatile int sound_level_0;
volatile int sound_level_1;

volatile enum player_state player_state;
volatile enum player_state decoder_state;
volatile enum player_state sound_state;

extern volatile uint32_t driver_lock;

static SNDBUF hBuff;

static int snd_format;
int sample_rate;

static uint32_t samples_written = 0;
double audio_base = -1.0;

double get_audio_base();

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

//    printf("sound version 0x%x\n", version);

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

void set_audio_volume(int left, int right)
{
    SetVolume(hBuff, left, right);
};

static uint64_t samples_lost;
static double  audio_delta;
static double  last_time_stamp;


double get_master_clock(void)
{
    double tstamp;

    GetTimeStamp(hBuff, &tstamp);
    return tstamp - audio_delta;
};

int decode_audio(AVCodecContext  *ctx, queue_t *qa)
{
    AVPacket   pkt;
    AVPacket    pkt_tmp;

    int         len;
    int         data_size=0;

    if( astream.count > AVCODEC_MAX_AUDIO_FRAME_SIZE*7)
        return -1;

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
//            if(audio_base == -1.0)
//            {
//                if (pkt.pts != AV_NOPTS_VALUE)
//                    audio_base = get_audio_base() * pkt.pts;
//                printf("audio base %f\n", audio_base);
//            };

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


static void sync_audio(SNDBUF hbuff, int buffsize)
{
    SND_EVENT   evnt;
    uint32_t    offset;
    double      time_stamp;

#ifdef BLACK_MAGIC_SOUND

    while( player_state != CLOSED)
    {
        GetNotify(&evnt);

        if(evnt.code != 0xFF000001)
        {
            printf("invalid event code %d\n\r", evnt.code);
            continue;
        }

        if(evnt.stream != hbuff)
        {
            printf("invalid stream %x hBuff= %x\n\r",
                    evnt.stream, hbuff);
            continue;
        }

        GetTimeStamp(hbuff, &time_stamp);
        audio_delta = time_stamp - last_time_stamp;

        offset = evnt.offset;

        mutex_lock(&astream.lock);
        {
            if(astream.count < buffsize)
            {
                memset(astream.buffer+astream.count,
                       0, buffsize-astream.count);
                astream.count = buffsize;
            };

            SetBuffer(hbuff, astream.buffer, offset, buffsize);
            samples_written+= buffsize/4;

            astream.count -= buffsize;
            if(astream.count)
                memcpy(astream.buffer, astream.buffer+buffsize, astream.count);
            mutex_unlock(&astream.lock);
        };
        break;
    };
#endif

};

int audio_thread(void *param)
{
    SND_EVENT evnt;

    int       buffsize;
    int      samples;
    int       err;
    char     *errstr;
    int       active;


    if((err = CreateBuffer(snd_format|PCM_RING,0, &hBuff)) != 0)
    {
        errstr = "Cannot create sound buffer\n\r";
        goto exit_whith_error;
    };

    SetVolume(hBuff,-1875,-1875);

    if((err = GetBufferSize(hBuff, &buffsize)) != 0)
    {
        errstr = "Cannot get buffer size\n\r";
        goto exit_whith_error;
    };

    resampler_size = buffsize = buffsize/2;

    samples = buffsize/4;

    while( player_state != CLOSED)
    {
        uint32_t  offset;
        double    event_stamp, wait_stamp;
        int       too_late = 0;

        switch(sound_state)
        {
            case PREPARE:

                mutex_lock(&astream.lock);
                    if(astream.count < buffsize*2)
                    {
                        memset(astream.buffer+astream.count,
                               0, buffsize*2-astream.count);
                        astream.count = buffsize*2;
                    };

                    SetBuffer(hBuff, astream.buffer, 0, buffsize*2);
                    astream.count -= buffsize*2;
                    if(astream.count)
                        memcpy(astream.buffer, astream.buffer+buffsize*2, astream.count);
                mutex_unlock(&astream.lock);

                SetTimeBase(hBuff, audio_base);

            case PAUSE_2_PLAY:
                GetTimeStamp(hBuff, &last_time_stamp);
//                printf("last audio time stamp %f\n", last_time_stamp);

                if((err = PlayBuffer(hBuff, 0)) !=0 )
                {
                    errstr = "Cannot play buffer\n\r";
                    goto exit_whith_error;
                };
                active = 1;
                sync_audio(hBuff, buffsize);
                sound_state = PLAY;
//                printf("render: set audio latency to %f\n", audio_delta);

                /* breaktrough */

            case PLAY:
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

                offset = evnt.offset;

                mutex_lock(&astream.lock);
                if(astream.count < buffsize)
                {
                    memset(astream.buffer+astream.count,
                           0, buffsize-astream.count);
                    astream.count = buffsize;
                };

                SetBuffer(hBuff, astream.buffer, offset, buffsize);

                {
                    double  val = 0;
                    int16_t *src = (int16_t*)astream.buffer;
                    int samples = buffsize/2;
                    int i;

                    for(i = 0, val = 0; i < samples/2; i++, src++)
                        if(val < abs(*src))
                            val= abs(*src); // * *src; 

                    sound_level_0 = val; //sqrt(val / (samples/2));
                    
                    for(i = 0, val = 0; i < samples/2; i++, src++)
                        if(val < abs(*src))
                            val= abs(*src); // * *src; 

                    sound_level_1 = val; //sqrt(val / (samples/2));
                    
 //                   printf("%d\n", sound_level);
                };

                samples_written+= buffsize/4;

                astream.count -= buffsize;
                if(astream.count)
                    memcpy(astream.buffer, astream.buffer+buffsize, astream.count);
                mutex_unlock(&astream.lock);
                break;

            case PLAY_2_STOP:
                if( active )
                {
                    ResetBuffer(hBuff, SND_RESET_ALL);
                    audio_base = -1.0;
                    active = 0;
                }
                sound_state = STOP;
                break;

            case PLAY_2_PAUSE:
                if( active )
                {
                    StopBuffer(hBuff);
                };
                sound_state = PAUSE;

            case PAUSE:
            case STOP:
                delay(1);
        };
    }

    StopBuffer(hBuff);
    DestroyBuffer(hBuff);

    return 0;

exit_whith_error:

    printf(errstr);
    return -1;

};

