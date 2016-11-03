#include <stdint.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <kos32sys.h>
#include "winlib/winlib.h"

#include "sound.h"
#include "fplay.h"

static struct decoder ffmpeg_decoder;
static struct decoder* init_ffmpeg_decoder(vst_t *vst);

static decoder_init_fn* decoders[] = {
    init_va_decoder,
    init_ffmpeg_decoder,
    NULL
};

static void fini_ffmpeg_decoder(vst_t *vst)
{
    av_frame_free(&vst->decoder->Frame);

    for(int i = 0; i < vst->decoder->nframes; i++)
    {
        vframe_t *vframe;
        vframe = &vst->decoder->vframes[i];
        avpicture_free(&vframe->picture);
    };
};

static struct decoder* init_ffmpeg_decoder(vst_t *vst)
{
    AVCodecContext *vCtx = vst->vCtx;
    struct decoder *decoder;
    vframe_t *vframe;
    int i, ret;

    decoder = &ffmpeg_decoder;

    decoder->Frame = av_frame_alloc();
    if(decoder->Frame == NULL)
        goto err_0;

    decoder->nframes = 4;

    for(i = 0; i < decoder->nframes; i++)
    {
        vframe = &decoder->vframes[i];

        ret = avpicture_alloc(&vframe->picture, vCtx->pix_fmt,
                               vCtx->width, vCtx->height);
        if ( ret != 0 )
            goto err_1;

        vframe->format = vCtx->pix_fmt;
        vframe->index  = i;
        list_add_tail(&vframe->list, &vst->input_list);
    };

    if(avcodec_open2(vCtx, vst->vCodec, NULL) < 0)
    {
        printf("Error while opening codec for input stream %d\n",
                vst->vStream);
        goto err_1;
    };

    decoder->name     = vst->vCodec->name;
    decoder->pix_fmt  = vCtx->pix_fmt;
    decoder->width    = vCtx->width;
    decoder->height   = vCtx->height;
    decoder->codec_id = vCtx->codec_id;
    decoder->profile  = vCtx->profile;
    decoder->fini     = fini_ffmpeg_decoder;
    return decoder;

err_1:
    for(i = i-1; i >= 0; i--)
    {
        vframe = &decoder->vframes[i];
        avpicture_free(&vframe->picture);
    };
err_0:
    return NULL;
}

static vframe_t *get_input_frame(vst_t *vst)
{
    vframe_t *vframe = NULL;

    mutex_lock(&vst->input_lock);
    if(!list_empty(&vst->input_list))
    {
        vframe = list_first_entry(&vst->input_list, vframe_t, list);
        list_del(&vframe->list);
    }
    mutex_unlock(&vst->input_lock);

    return vframe;
}

static void put_output_frame(vst_t *vst, vframe_t *vframe)
{
    mutex_lock(&vst->output_lock);
    if(list_empty(&vst->output_list))
        list_add_tail(&vframe->list, &vst->output_list);
    else
    {
        vframe_t *cur;

        cur = list_first_entry(&vst->output_list,vframe_t,list);
        if(vframe->pts < cur->pts)
            list_add_tail(&vframe->list, &vst->output_list);
        else
        {
            list_for_each_entry_reverse(cur,&vst->output_list,list)
            {
                if(vframe->pts > cur->pts)
                {
                    list_add(&vframe->list, &cur->list);
                    break;
                };
            };
        };
    };
    vst->frames_count++;
    mutex_unlock(&vst->output_lock);
};

int decode_video(vst_t* vst)
{
    struct decoder* decoder = vst->decoder;
    double     pts;
    AVPacket   pkt;

    int frameFinished;

    if(decoder->active_frame == NULL)
        decoder->active_frame = get_input_frame(vst);

    if(decoder->active_frame == NULL)
        return -1;

    if( get_packet(&vst->q_video, &pkt) == 0 )
        return 0;

    frameFinished = 0;

    mutex_lock(&vst->gpu_lock);

    if(avcodec_decode_video2(vst->vCtx, decoder->Frame, &frameFinished, &pkt) <= 0)
        printf("video decoder error\n");

    if(frameFinished)
    {
        vframe_t  *vframe = decoder->active_frame;
        AVPicture *dst_pic;

        if(decoder->is_hw)
            pts = pkt.pts;
        else
            pts = av_frame_get_best_effort_timestamp(decoder->Frame);

        pts*= av_q2d(vst->video_time_base);

        dst_pic = &vframe->picture;

        if(vframe->is_hw_pic == 0)
            av_image_copy(dst_pic->data, dst_pic->linesize,
                          (const uint8_t**)decoder->Frame->data,
                          decoder->Frame->linesize, vst->vCtx->pix_fmt, vst->vCtx->width, vst->vCtx->height);
        else
            va_create_planar(vst, vframe);

        vframe->pts = pts*1000.0;
        vframe->pkt_pts = pkt.pts*av_q2d(vst->video_time_base)*1000.0;
        vframe->ready = 1;

        put_output_frame(vst, vframe);

//        printf("decoded index: %d pts: %f pkt_pts %f pkt_dts %f\n",
//               vst->dfx, vst->vframe[vst->dfx].pts,
//               vst->vframe[vst->dfx].pkt_pts, vst->vframe[vst->dfx].pkt_dts);

        decoder->active_frame = NULL;
    };
    av_frame_unref(decoder->Frame);
    mutex_unlock(&vst->gpu_lock);

    av_free_packet(&pkt);

    return 1;
}

int init_video_decoder(vst_t *vst)
{
    decoder_init_fn **init_fn;
    AVCodecContext *vCtx = vst->vCtx;
    int i;

    vst->vCodec = avcodec_find_decoder(vCtx->codec_id);

    if(vst->vCodec == NULL)
    {
        printf("Unsupported codec with id %d for input stream %d\n",
        vst->vCtx->codec_id, vst->vStream);
        return -1;
    }

    for(init_fn = decoders; init_fn != NULL; init_fn++)
    {
        vst->decoder = (*init_fn)(vst);
        if(vst->decoder != NULL)
        {
            printf("%dx%d %s %s%s decoder\n",
                    vst->decoder->width, vst->decoder->height,
                    av_get_pix_fmt_name(vst->decoder->pix_fmt),
                    vst->decoder->is_hw == 0 ? "ffmpeg ":"vaapi ",
                    vst->decoder->name);

            return 0;
        };
    }

    return -1;
}

