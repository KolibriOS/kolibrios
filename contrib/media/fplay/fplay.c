
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
#include "../winlib/winlib.h"

#include "sound.h"
#include "fplay.h"

volatile enum player_state player_state  = STOP;
volatile enum player_state decoder_state = PREPARE;
volatile enum player_state sound_state   = STOP;

uint32_t win_width, win_height;

void decoder();

AVFormatContext *pFormatCtx;
AVCodecContext  *pCodecCtx;
AVCodecContext  *aCodecCtx;
AVCodec         *pCodec;
AVCodec         *aCodec;
AVFrame         *pFrame;
int             videoStream;
int             audioStream;

int             have_sound = 0;

uint8_t     *decoder_buffer;
extern int resampler_size;

extern int sample_rate;
char *movie_file;

void flush_video();

queue_t  q_video;
queue_t  q_audio;
int64_t  rewind_pos;

int64_t stream_duration;

extern double audio_base;

double get_audio_base()
{

  return (double)av_q2d(pFormatCtx->streams[audioStream]->time_base)*1000;

};



int main( int argc, char *argv[])
{
    int i;
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

#if 0
    {
        int fd, i;
        char *buff;
        uint32_t start, stop;

        fd = open(movie_file,O_RDONLY);

        if(fd < 0)
            return 0;

        buff = user_alloc(65536);
        memset(buff, 0, 65536);

        start = get_tick_count();

        for(i = 0; i < 1024*1024*1024; i+=65536)
        {
            if(read(fd, buff, 65536) < 0)
                break;

        };
        stop = get_tick_count();

        printf("average speed %d Kbytes/c\n", ((i/1024)*100)/(stop-start));
    };
    return 0;
};

#else

    av_log_set_level(AV_LOG_FATAL);

    avcodec_register_all();
    avdevice_register_all();
    av_register_all();

//    init_pixlib(HW_BIT_BLIT|HW_TEX_BLIT);

    if( avformat_open_input(&pFormatCtx, movie_file, NULL, NULL) < 0)
    {
        printf("Cannot open file %s\n\r", movie_file);
        return -1; // Couldn't open file
    };

    pFormatCtx->flags |= AVFMT_FLAG_GENPTS;

  // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
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


//    __asm__ __volatile__("int3");

//  dump_format(pFormatCtx, 0, argv[1], 0);

//    stream_duration = 1000.0 * pFormatCtx->duration * av_q2d(AV_TIME_BASE_Q);
    stream_duration = pFormatCtx->duration;

    printf("duration %f\n", (double)stream_duration);
   // Find the first video stream
    videoStream=-1;
    audioStream=-1;
    for(i=0; i < pFormatCtx->nb_streams; i++)
    {
//        pFormatCtx->streams[i]->discard = AVDISCARD_ALL;

        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO
            && videoStream < 0)
        {
            videoStream=i;
            video_time_base = pFormatCtx->streams[i]->time_base;
            if(stream_duration == 0)
//                stream_duration = 1000.0 *
//                              pFormatCtx->streams[i]->duration *
//                              av_q2d(pFormatCtx->streams[i]->time_base);
               stream_duration = pFormatCtx->streams[i]->duration;

        }
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO &&
            audioStream < 0)
        {
            audioStream=i;
            if(stream_duration == 0)
//                stream_duration = 1000.0 *
//                              pFormatCtx->streams[i]->duration *
//                              av_q2d(pFormatCtx->streams[i]->time_base);
               stream_duration = pFormatCtx->streams[i]->duration;

        }
    }

    if(videoStream==-1)
    {
        printf("Video stream not detected\n\r");
        return -1; // Didn't find a video stream
    };

#if 0
    {
        AVPacket  packet;
        int psize = 0;
        uint32_t start, stop;

        int err;
        start = get_tick_count();

        while(psize < 1024*1024*1024)
        {
            err = av_read_frame(pFormatCtx, &packet);
            if(err != 0)
                break;
            psize+= packet.size;
            av_free_packet(&packet);
        };

        stop = get_tick_count();

        printf("average speed %d Kbytes/c\n", ((psize/1024)*100)/(stop-start));

        return 1;
    };
};
#else


  //   __asm__ __volatile__("int3");

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
    aCodecCtx=pFormatCtx->streams[audioStream]->codec;

  // Find the decoder for the video stream

//    init_hw_context(pCodecCtx);

    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);

//    printf("ctx->pix_fmt %d\n", pCodecCtx->pix_fmt);

    if(pCodec==NULL) {
        printf("Unsupported codec with id %d for input stream %d\n",
        pCodecCtx->codec_id, videoStream);
        return -1; // Codec not found
    }

    if(avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("Error while opening codec for input stream %d\n",
                videoStream);
        return -1; // Could not open codec
    };

//    printf("ctx->pix_fmt %d\n", pCodecCtx->pix_fmt);


    if (aCodecCtx->channels > 0)
            aCodecCtx->request_channels = FFMIN(2, aCodecCtx->channels);
    else
            aCodecCtx->request_channels = 2;

    aCodec = avcodec_find_decoder(aCodecCtx->codec_id);

    if(aCodec)
    {
        if(avcodec_open2(aCodecCtx, aCodec, NULL) >= 0 )
        {
            WAVEHEADER       whdr;
            int fmt;
            int channels;

            printf("audio stream rate %d channels %d format %d\n",
            aCodecCtx->sample_rate, aCodecCtx->channels, aCodecCtx->sample_fmt );
            whdr.riff_id = 0x46464952;
            whdr.riff_format = 0x45564157;
            whdr.wFormatTag = 0x01;
            whdr.nSamplesPerSec = aCodecCtx->sample_rate;
            whdr.nChannels = 2;
            whdr.wBitsPerSample = 16;

            sample_rate = aCodecCtx->sample_rate;

            fmt = test_wav(&whdr);

            if( init_audio(fmt) )
            {
                decoder_buffer = (uint8_t*)av_mallocz(192000*2+64);
                if( decoder_buffer != NULL )
                {
                    astream.lock   = 0;
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

    if( !init_video(pCodecCtx))
        return 0;

//    __asm__ __volatile__("int3");

    decoder();

  // Free the YUV frame
    av_free(pFrame);


//__asm__ __volatile__("int3");

  // Close the codec
 // avcodec_close(pCodecCtx);

  // Close the video file
 // av_close_input_file(pFormatCtx);

//__asm__ __volatile__("int3");

    return 0;
}


static int load_frame()
{
    AVPacket  packet;
    int err;

    err = av_read_frame(pFormatCtx, &packet);
    if( err == 0)
    {
        if(packet.stream_index==videoStream)
            put_packet(&q_video, &packet);
        else if( (packet.stream_index == audioStream) &&
                  (have_sound != 0) )
        {
            put_packet(&q_audio, &packet);
            if(audio_base == -1.0)
            {
                if (packet.pts != AV_NOPTS_VALUE)
                    audio_base = get_audio_base() * packet.pts;
//                    printf("audio base %f\n", audio_base);
            };
        }
        else av_free_packet(&packet);
    }
    else if (err != AVERROR_EOF)
        printf("av_read_frame: error %x\n", err);

    return err;
}



static int fill_queue()
{
    int err = 0;
    AVPacket  packet;

//        __asm__ __volatile__("int3");

    while( (q_video.size < 4*1024*1024) &&
            !err )
        err = load_frame();

    return err;

};


static void flush_all()
{
    AVPacket  packet;

    avcodec_flush_buffers(pCodecCtx);
    avcodec_flush_buffers(aCodecCtx);
    while( get_packet(&q_video, &packet) != 0)
        av_free_packet(&packet);

    while( get_packet(&q_audio, &packet)!= 0)
        av_free_packet(&packet);

    flush_video();

    astream.count = 0;
};

void decoder()
{
    int       eof;
    AVPacket  packet;
    int       ret, vret, aret;

    int64_t   min_pos, max_pos;

    while( player_state != CLOSED )
    {
        int err;

//        __asm__ __volatile__("int3");

        switch(decoder_state)
        {
            case PREPARE:
                eof = fill_queue();

                do
                {
                    if( (q_video.size < 4*1024*1024) &&
                        (eof == 0) )
                    {
                        eof = load_frame();
                    }
                    decode_video(pCodecCtx, &q_video);
                    ret = decode_audio(aCodecCtx, &q_audio);
                }while(astream.count < resampler_size*2 &&
                       ret == 1);

                sound_state   = PREPARE;
                decoder_state = PLAY;
                player_state  = PLAY;

            case PLAY:
                if( (q_video.size < 4*1024*1024) &&
                    (eof == 0) )
                {
                    eof = load_frame();
                }
                vret = decode_video(pCodecCtx, &q_video);
                aret = decode_audio(aCodecCtx, &q_audio);
                ret = vret | aret;

                if( eof && !ret)
                {
                    decoder_state = STOP;
                    continue;
                };

                if( (vret & aret) == -1)
                {
                    if( (q_video.size < 4*1024*1024) &&
                        (eof == 0) )
                    {
                        eof = load_frame();
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

                flush_all();

                if (pFormatCtx->start_time != AV_NOPTS_VALUE)
                    rewind_pos = pFormatCtx->start_time;
                else
                    rewind_pos = 0;

                ret = avformat_seek_file(pFormatCtx, -1, INT64_MIN,
                                         rewind_pos, INT64_MAX, 0);

                decoder_state = STOP;
                break;

            case REWIND:
                while(sound_state != STOP)
                    yield();

                flush_all();
                int opts = 0;
                if(rewind_pos < 0)
                {
                    rewind_pos = -rewind_pos;
                    opts = AVSEEK_FLAG_BACKWARD;
                };

                if (pFormatCtx->start_time != AV_NOPTS_VALUE)
                    rewind_pos += pFormatCtx->start_time;

//                printf("rewind %8"PRId64"\n", rewind_pos);
                min_pos = rewind_pos - 1000000;
                max_pos = rewind_pos + 1000000;

                ret = avformat_seek_file(pFormatCtx, -1, INT64_MIN,
                                         rewind_pos, INT64_MAX, 0);

//                ret = avformat_seek_file(pFormatCtx, -1, min_pos,
//                                         rewind_pos, max_pos, opts);
//            __asm__ __volatile__("int3");

                if (ret < 0)
                {
                    printf("could not seek to position %f\n",
                            (double)rewind_pos / AV_TIME_BASE);
                }

//                printf("restart\n");
                decoder_state = PREPARE;
                break;
        }
    };

    ret = 1;

    while( (player_state != CLOSED) && ret)
    {
        ret =  decode_video(pCodecCtx, &q_video);
        ret |= decode_audio(aCodecCtx, &q_audio);
        delay(1);
    };
    delay(50);
    player_state = CLOSED;
    delay(300);
};
#endif
#endif
