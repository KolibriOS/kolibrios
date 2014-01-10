
#include <stdint.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "libswresample/swresample.h"

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
    static struct SwrContext *swr_ctx;
    static int64_t src_layout;
    static int src_freq;
    static int src_channels;
    static enum AVSampleFormat src_fmt = -1;
    static AVFrame *aFrame;

    AVPacket   pkt;
    AVPacket    pkt_tmp;
    int64_t dec_channel_layout;
    int len, len2;
    int got_frame;
    int data_size;


    if( astream.count > 192000*2)
        return -1;

    if( get_packet(qa, &pkt) == 0 )
        return 0;

 //          __asm__("int3");

    if (!aFrame)
    {
        if (!(aFrame = avcodec_alloc_frame()))
            return -1;
    } else
        avcodec_get_frame_defaults(aFrame);

    pkt_tmp = pkt;

    while(pkt_tmp.size > 0)
    {
        data_size = 192000;

//        len = avcodec_decode_audio3(ctx,(int16_t*)decoder_buffer,
//                                   &data_size, &pkt_tmp);
        got_frame = 0;
        len = avcodec_decode_audio4(ctx, aFrame, &got_frame, &pkt_tmp);

        if(len >= 0 && got_frame)
        {
            char *samples;
            int ch, plane_size;
            int planar    = av_sample_fmt_is_planar(ctx->sample_fmt);
            int data_size = av_samples_get_buffer_size(&plane_size, ctx->channels,
                                                   aFrame->nb_samples,
                                                   ctx->sample_fmt, 1);

//            if(audio_base == -1.0)
//            {
//                if (pkt.pts != AV_NOPTS_VALUE)
//                    audio_base = get_audio_base() * pkt.pts;
//                printf("audio base %f\n", audio_base);
//            };

            pkt_tmp.data += len;
            pkt_tmp.size -= len;

            dec_channel_layout =
                (aFrame->channel_layout && aFrame->channels == av_get_channel_layout_nb_channels(aFrame->channel_layout)) ?
                aFrame->channel_layout : av_get_default_channel_layout(aFrame->channels);

            if (aFrame->format          != src_fmt     ||
                dec_channel_layout      != src_layout  ||
                aFrame->sample_rate     != src_freq    ||
                !swr_ctx)
            {
                swr_free(&swr_ctx);
                swr_ctx = swr_alloc_set_opts(NULL, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16,
                                             aFrame->sample_rate, dec_channel_layout,aFrame->format,
                                             aFrame->sample_rate, 0, NULL);
                if (!swr_ctx || swr_init(swr_ctx) < 0)
                {
                    printf("Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                        aFrame->sample_rate,   av_get_sample_fmt_name(aFrame->format), (int)aFrame->channels,
                        aFrame->sample_rate, av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), 2);
                    break;
                }

                src_layout   = dec_channel_layout;
                src_channels = aFrame->channels;
                src_freq     = aFrame->sample_rate;
                src_fmt      = aFrame->format;
            };

            if (swr_ctx)
            {
                const uint8_t **in = (const uint8_t **)aFrame->extended_data;
                uint8_t *out[] = {decoder_buffer};
                int out_count = 192000 * 3 / 2 / av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);
                len2 = swr_convert(swr_ctx, out, out_count, in, aFrame->nb_samples);
                if (len2 < 0) {
                    printf("swr_convert() failed\n");
                    break;
                }
                if (len2 == out_count) {
                    printf("warning: audio buffer is probably too small\n");
                    swr_init(swr_ctx);
                }
                data_size = len2 * 2 * av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);

                mutex_lock(&astream.lock);

                samples = astream.buffer+astream.count;

                memcpy(samples, decoder_buffer, data_size);
/*
            memcpy(samples, aFrame->extended_data[0], plane_size);

            if (planar && ctx->channels > 1)
            {
                uint8_t *out = ((uint8_t *)samples) + plane_size;
                for (ch = 1; ch < ctx->channels; ch++)
                {
                    memcpy(out, aFrame->extended_data[ch], plane_size);
                    out += plane_size;
                }
            }
*/
                astream.count += data_size;
                mutex_unlock(&astream.lock);
            };
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

