
#include <stdint.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include "../winlib/winlib.h"

#include "sound.h"
#include "fplay.h"

volatile enum player_state player_state;

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

    if(argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }

    movie_file = argv[1];
    /* register all codecs, demux and protocols */

//    av_log_set_level(AV_LOG_INFO);

    avcodec_register_all();
    avdevice_register_all();
    av_register_all();

    if( avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) < 0)
    {
        printf("Cannot open file %s\n\r", argv[1]);
        return -1; // Couldn't open file
    };

    pFormatCtx->flags |= AVFMT_FLAG_GENPTS;

  // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    {
        printf("Cannot find streams\n\r");
        return -1;
    };

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
    }

    player_state = PLAY_INIT;

 //   __asm__ __volatile__("int3");

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;
    aCodecCtx=pFormatCtx->streams[audioStream]->codec;

  // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
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

            printf("audio stream rate %d channels %d\n",
            aCodecCtx->sample_rate, aCodecCtx->channels);

            whdr.riff_id = 0x46464952;
            whdr.riff_format = 0x45564157;
            whdr.wFormatTag = 0x01;
            whdr.nSamplesPerSec = aCodecCtx->sample_rate;
            whdr.nChannels = aCodecCtx->channels;
            whdr.wBitsPerSample = 16;

            sample_rate = aCodecCtx->sample_rate;

            fmt = test_wav(&whdr);

            if( init_audio(fmt) )
            {
                decoder_buffer = (uint8_t*)av_mallocz(AVCODEC_MAX_AUDIO_FRAME_SIZE*2+64);
                if( decoder_buffer != NULL )
                {
                    astream.lock   = 0;
                    astream.count  = 0;
                    astream.buffer = (char *)av_mallocz(AVCODEC_MAX_AUDIO_FRAME_SIZE*8);
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


static int fill_queue()
{
    int eof = 0;
    AVPacket  packet;

    while( !eof)
    {
        int err;

//        __asm__ __volatile__("int3");

        if(q_video.size+q_audio.size < 2*1024*1024)
        {
            err = av_read_frame(pFormatCtx, &packet);
            if( err < 0)
            {
                eof = 1;
                if (err != AVERROR_EOF)
                    printf("av_read_frame: error %x\n", err);
                break;
            }
            if(packet.stream_index==videoStream)
            {
                put_packet(&q_video, &packet);
            }
            else if( (packet.stream_index == audioStream) &&
                 (have_sound != 0) )
            {
                put_packet(&q_audio, &packet);
                if(audio_base == -1.0)
                {
                    if (packet.dts != AV_NOPTS_VALUE)
                        audio_base = get_audio_base() * packet.dts;
//                    printf("audio base %f\n", audio_base);
                };
            }
            else
            {
                av_free_packet(&packet);
            };
        }
        else break;
    };

    return eof;

};


void decoder()
{
    int       eof;
    AVPacket  packet;
    int       ret;

    eof = fill_queue();

    while( player_state != CLOSED && !eof)
    {
        int err;

//        __asm__ __volatile__("int3");

        if( player_state == PAUSE )
        {
            delay(1);
            continue;
        };

        if( player_state == REWIND )
        {
 //           int64_t timestamp = 0;
 //           int stream_index = av_find_default_stream_index(pFormatCtx);

 //           __asm__ __volatile__("int3");

            if (pFormatCtx->start_time != AV_NOPTS_VALUE)
                rewind_pos += pFormatCtx->start_time;

            printf("rewind %8"PRId64"\n", rewind_pos);
            
            ret = avformat_seek_file(pFormatCtx, -1, INT64_MIN,
                                     rewind_pos, INT64_MAX, 0);
//            ret = avformat_seek_file(pFormatCtx, -1, 0,
//                                 0, INT64_MAX, 0);
//            __asm__ __volatile__("int3");

            if (ret < 0)
            {
                printf("could not seek to position %f\n",
                        (double)rewind_pos / AV_TIME_BASE);
            }
            else
            {
                avcodec_flush_buffers(pCodecCtx);
                avcodec_flush_buffers(aCodecCtx);

                while( get_packet(&q_video, &packet) != 0)
                    av_free_packet(&packet);

                while( get_packet(&q_audio, &packet)!= 0)
                    av_free_packet(&packet);
                audio_base = -1.0;

  //              __asm__ __volatile__("int3");

                eof = fill_queue();
            };
            yield();

            flush_video();

            player_state = REWIND_2_PLAY;
            printf("restart\n");
            continue;
        };

        if(q_video.size+q_audio.size < 4*1024*1024)
        {
            err = av_read_frame(pFormatCtx, &packet);
            if( err < 0)
            {
                eof = 1;
                if (err != AVERROR_EOF)
                    printf("av_read_frame: error %x\n", err);
                continue;
            }
            if(packet.stream_index==videoStream)
            {
                put_packet(&q_video, &packet);
            }
            else if( (packet.stream_index == audioStream) &&
                 (have_sound != 0) )
            {
                put_packet(&q_audio, &packet);
            }
            else
            {
                av_free_packet(&packet);
            };
            decode_video(pCodecCtx, &q_video);
            decode_audio(aCodecCtx, &q_audio);
            continue;
        };
        decode_video(pCodecCtx, &q_video);
        decode_audio(aCodecCtx, &q_audio);
        delay(1);
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

