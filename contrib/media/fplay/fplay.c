
#include <stdint.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <kos32sys.h>
#include "winlib/winlib.h"

#include "sound.h"
#include "fplay.h"

#ifdef HAVE_VAAPI
int va_check_codec_support(enum AVCodecID id);
#endif

volatile enum player_state player_state  = STOP;
volatile enum player_state decoder_state = PREPARE;
volatile enum player_state sound_state   = STOP;

uint32_t win_width, win_height;


AVFrame *pFrame;

int      have_sound = 0;

uint8_t  *decoder_buffer;
extern int resampler_size;

extern int sample_rate;
char *movie_file;

void flush_video(vst_t* vst);


int64_t  rewind_pos;

int64_t stream_duration;

int threads_running = DECODER_THREAD;

extern double audio_base;


double get_audio_base(vst_t* vst)
{
    return (double)av_q2d(vst->fCtx->streams[vst->aStream]->time_base)*1000;
};


int main( int argc, char *argv[])
{
    static vst_t vst;
    int i, ret;
    char *file_name, *dot;

    if(argc < 2)
    {
        movie_file = get_moviefile();
        if(movie_file == NULL)
        {
            printf("Please provide a movie file\n");
            return -1;
        }
    }
    else movie_file = argv[1];

    /* register all codecs, demux and protocols */

    av_log_set_level(AV_LOG_FATAL);

    avcodec_register_all();
    avdevice_register_all();
    av_register_all();

    if( avformat_open_input(&vst.fCtx, movie_file, NULL, NULL) < 0)
    {
        printf("Cannot open file %s\n\r", movie_file);
        return -1; // Couldn't open file
    };

    vst.fCtx->flags |= AVFMT_FLAG_GENPTS;

  // Retrieve stream information
    if(avformat_find_stream_info(vst.fCtx, NULL) < 0)
    {
        printf("Cannot find streams\n\r");
        return -1;
    };

    file_name = strrchr(movie_file,'/')+1;
    dot = strrchr(file_name,'.');
    if(dot)
    {
        movie_file = malloc(dot-file_name+1);
        memcpy(movie_file, file_name, dot-file_name);
        movie_file[dot-file_name] = 0;
    }
    else movie_file = file_name;

    stream_duration = vst.fCtx->duration;

   // Find the first video stream
    vst.vStream = -1;
    vst.aStream = -1;

    for(i=0; i < vst.fCtx->nb_streams; i++)
    {
        if(vst.fCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO
            && vst.vStream < 0)
        {
            vst.vStream = i;
            video_time_base = vst.fCtx->streams[i]->time_base;
            if(stream_duration == 0)
               stream_duration = vst.fCtx->streams[i]->duration;
        }

        if(vst.fCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO &&
           vst.aStream < 0)
        {
            vst.aStream = i;
            if(stream_duration == 0)
               stream_duration = vst.fCtx->streams[i]->duration;
        }
    }

    if(vst.vStream==-1)
    {
        printf("Video stream not detected\n\r");
        return -1; // Didn't find a video stream
    };

  //   __asm__ __volatile__("int3");

    // Get a pointer to the codec context for the video stream
    vst.vCtx = vst.fCtx->streams[vst.vStream]->codec;
    vst.aCtx = vst.fCtx->streams[vst.aStream]->codec;

    vst.vCodec = avcodec_find_decoder(vst.vCtx->codec_id);
    printf("codec id %x name %s\n",vst.vCtx->codec_id, vst.vCodec->name);
    printf("ctx->pix_fmt %d\n", vst.vCtx->pix_fmt);

    if(vst.vCodec == NULL)
    {
        printf("Unsupported codec with id %d for input stream %d\n",
        vst.vCtx->codec_id, vst.vStream);
        return -1; // Codec not found
    }

    if(fplay_init_context(&vst))
        return -1;

    if(avcodec_open2(vst.vCtx, vst.vCodec, NULL) < 0)
    {
        printf("Error while opening codec for input stream %d\n",
                vst.vStream);
        return -1; // Could not open codec
    };

    printf("ctx->pix_fmt %d\n", vst.vCtx->pix_fmt);

    mutex_init(&vst.q_video.lock);
    mutex_init(&vst.q_audio.lock);
    mutex_init(&vst.gpu_lock);

    if (vst.aCtx->channels > 0)
        vst.aCtx->request_channels = FFMIN(2, vst.aCtx->channels);
    else
        vst.aCtx->request_channels = 2;

    vst.aCodec = avcodec_find_decoder(vst.aCtx->codec_id);

    if(vst.aCodec)
    {
        if(avcodec_open2(vst.aCtx, vst.aCodec, NULL) >= 0 )
        {
            WAVEHEADER       whdr;
            int fmt;
            int channels;

            printf("audio stream rate %d channels %d format %d\n",
            vst.aCtx->sample_rate, vst.aCtx->channels, vst.aCtx->sample_fmt );
            whdr.riff_id = 0x46464952;
            whdr.riff_format = 0x45564157;
            whdr.wFormatTag = 0x01;
            whdr.nSamplesPerSec = vst.aCtx->sample_rate;
            whdr.nChannels = 2;
            whdr.wBitsPerSample = 16;

            sample_rate = vst.aCtx->sample_rate;

            fmt = test_wav(&whdr);

            if( init_audio(fmt) )
            {
                decoder_buffer = (uint8_t*)av_mallocz(192000*2+64);
                if( decoder_buffer != NULL )
                {
                    mutex_init(&astream.lock);
                    astream.count  = 0;
                    astream.buffer = (char *)av_mallocz(192000*3);
                    if( astream.buffer != NULL )
                        have_sound = 1;
                    else
                        av_free(decoder_buffer);
                }
                if( have_sound == 0)
                {
                        printf("Not enough memory for audio buffers\n");
                }
            }
        }
        else printf("Cannot open audio codec\n\r");
    }
    else printf("Unsupported audio codec!\n");

    if(!init_video(&vst))
        return 0;

    decoder(&vst);

  // Free the YUV frame
    av_free(pFrame);


//__asm__ __volatile__("int3");

    while( threads_running &
           (AUDIO_THREAD | VIDEO_THREAD))
           delay(1);

    if(astream.lock.handle)
        mutex_destroy(&astream.lock);

    mutex_destroy(&vst.q_video.lock);
    mutex_destroy(&vst.q_audio.lock);

    return 0;
}


static int load_frame(vst_t *vst)
{
    AVPacket  packet;
    int err;

    err = av_read_frame(vst->fCtx, &packet);
    if( err == 0)
    {
        if(packet.stream_index == vst->vStream)
            put_packet(&vst->q_video, &packet);
        else if( (packet.stream_index == vst->aStream) &&
                  (have_sound != 0) )
        {
            put_packet(&vst->q_audio, &packet);
            if(audio_base == -1.0)
            {
                if (packet.pts != AV_NOPTS_VALUE)
                    audio_base = get_audio_base(vst) * packet.pts;
//                    printf("audio base %f\n", audio_base);
            };
        }
        else av_free_packet(&packet);
    }
    else if (err != AVERROR_EOF)
        printf("av_read_frame: error %x\n", err);

    return err;
}



static int fill_queue(vst_t* vst)
{
    int err = 0;
    AVPacket  packet;

    while( (vst->q_video.size < 4*1024*1024) && !err )
        err = load_frame(vst);

    return err;
};


static void flush_all(vst_t* vst)
{
    AVPacket  packet;

    avcodec_flush_buffers(vst->vCtx);
    avcodec_flush_buffers(vst->aCtx);
    while( get_packet(&vst->q_video, &packet) != 0)
        av_free_packet(&packet);

    while( get_packet(&vst->q_audio, &packet)!= 0)
        av_free_packet(&packet);

    flush_video(vst);

    astream.count = 0;
};

void decoder(vst_t* vst)
{
    int       eof;
    AVPacket  packet;
    int       ret, vret, aret;

    int64_t   min_pos, max_pos;

//    av_log_set_level(AV_LOG_DEBUG);

    while( player_state != CLOSED )
    {
        int err;

        switch(decoder_state)
        {
            case PREPARE:
                eof = fill_queue(vst);

                do
                {
                    if( (vst->q_video.size < 4*1024*1024) &&
                        (eof == 0) )
                    {
                        eof = load_frame(vst);
                    }
                    decode_video(vst);
                    ret = decode_audio(vst->aCtx, &vst->q_audio);
                }while(astream.count < resampler_size*2 &&
                       ret == 1);

                sound_state   = PREPARE;
                decoder_state = PLAY;
                player_state  = PLAY;

            case PLAY:
                if( (vst->q_video.size < 4*1024*1024) &&
                    (eof == 0) )
                {
                    eof = load_frame(vst);
                }
                vret = decode_video(vst);
                aret = decode_audio(vst->aCtx, &vst->q_audio);
                ret = vret | aret;

                if( eof && !ret)
                {
                    decoder_state = STOP;
                    continue;
                };

                if( (vret & aret) == -1)
                {
                    if( (vst->q_video.size < 4*1024*1024) &&
                        (eof == 0) )
                    {
                        eof = load_frame(vst);
                        yield();
                        continue;
                    };
                    delay(1);
                    continue;
                }

                yield();
                continue;

            case STOP:
                delay(1);
                continue;


            case PLAY_2_STOP:
                while(sound_state != STOP)
                    delay(1);

                flush_all(vst);

                if (vst->fCtx->start_time != AV_NOPTS_VALUE)
                    rewind_pos = vst->fCtx->start_time;
                else
                    rewind_pos = 0;

                ret = avformat_seek_file(vst->fCtx, -1, INT64_MIN,
                                         rewind_pos, INT64_MAX, 0);

                decoder_state = STOP;
                break;

            case REWIND:
                while(sound_state != STOP)
                    yield();

                flush_all(vst);
                int opts = 0;
                if(rewind_pos < 0)
                {
                    rewind_pos = -rewind_pos;
                    opts = AVSEEK_FLAG_BACKWARD;
                };

                if (vst->fCtx->start_time != AV_NOPTS_VALUE)
                    rewind_pos += vst->fCtx->start_time;

//                printf("rewind %8"PRId64"\n", rewind_pos);
                min_pos = rewind_pos - 1000000;
                max_pos = rewind_pos + 1000000;

                ret = avformat_seek_file(vst->fCtx, -1, INT64_MIN,
                                         rewind_pos, INT64_MAX, 0);
                if (ret < 0)
                {
                    printf("could not seek to position %f\n",
                            (double)rewind_pos / AV_TIME_BASE);
                }
                decoder_state = PREPARE;
                break;
        }
    };
};

