
#include <stdint.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "sound.h"
#include "fplay.h"

volatile uint32_t status = 1;

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

int main( int argc, char *argv[])
{
    int i;

    if(argc < 2) {
        printf("Please provide a movie file\n");
        return -1;
    }

    /* register all codecs, demux and protocols */

    avcodec_register_all();
    avdevice_register_all();
    av_register_all();


  // Open video file
    if(av_open_input_file(&pFormatCtx, argv[1], NULL, 0, NULL)!=0)
    {
        printf("Cannot open file %s\n\r", argv[1]);
        return -1; // Couldn't open file
    };

//    __asm__ __volatile__("int3");

  // Retrieve stream information
    if(av_find_stream_info(pFormatCtx)<0)
    {
        printf("Cannot find streams\n\r");
        return -1;
    };

 //   __asm__ __volatile__("int3");

 // dump_format(pFormatCtx, 0, argv[1], 0);

   // Find the first video stream
    videoStream=-1;
    audioStream=-1;
    for(i=0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO
            && videoStream < 0)
        {
            videoStream=i;
            video_time_base = pFormatCtx->streams[i]->time_base;

        }
        if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_AUDIO &&
            audioStream < 0)
        {
            audioStream=i;
        }
    }

    if(videoStream==-1)
    {
        printf("Video stream not detected\n\r");
        return -1; // Didn't find a video stream
    }


    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;

    aCodecCtx=pFormatCtx->streams[audioStream]->codec;

  // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        printf("Unsupported video codec!\n");
        return -1; // Codec not found
    }
  // Open codec
    if(avcodec_open(pCodecCtx, pCodec) < 0)
    {
        printf("Cannot open video codec\n\r");
        return -1; // Could not open codec
    };

    if (aCodecCtx->channels > 0)
            aCodecCtx->request_channels = FFMIN(2, aCodecCtx->channels);
    else
            aCodecCtx->request_channels = 2;

    aCodec = avcodec_find_decoder(aCodecCtx->codec_id);

    if(aCodec)
    {
        if(avcodec_open(aCodecCtx, aCodec) >= 0 )
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
                decoder_buffer = (uint8_t*)av_mallocz(AVCODEC_MAX_AUDIO_FRAME_SIZE);
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

  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture

 //   __asm__ __volatile__("int3");

    decoder();

    status = 0;


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

void decoder()
{
    AVPacket   packet;

    while(av_read_frame(pFormatCtx, &packet) >=0 )
    {
        if(packet.stream_index==videoStream)
        {
            decode_video(pCodecCtx, &packet);
        }
        else if( (packet.stream_index == audioStream) &&
                 (have_sound != 0) )
        {
            uint8_t    *audio_data;
            int         audio_size;
            int         len;
            int         data_size=0;

            audio_data = packet.data;
            audio_size = packet.size;

            while(audio_size > 0)
            {
               data_size = AVCODEC_MAX_AUDIO_FRAME_SIZE;

               len = avcodec_decode_audio2(aCodecCtx,(int16_t*)decoder_buffer,
                                           &data_size, audio_data, audio_size);

               if(len >= 0)
               {
                    audio_data += len;
                    audio_size -= len;

                    while((astream.count + data_size) >
                           AVCODEC_MAX_AUDIO_FRAME_SIZE*8)
                    {
                        yield();
                    }
                    spinlock_lock(&astream.lock);
                    memcpy(astream.buffer+astream.count, decoder_buffer, data_size);
                    astream.count += data_size;
                    spinlock_unlock(&astream.lock);
               }
               else audio_size = 0;
            }
        }
    // Free the packet that was allocated by av_read_frame
        av_free_packet(&packet);
    };
};


__int64 _lseeki64(int fd, __int64 offset,  int origin )
{
    int off = offset;
    return lseek(fd, off, origin);
}



