
#include <stdint.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <kos32sys.h>
#include "winlib/winlib.h"

#include "sound.h"
#include "fplay.h"

volatile enum player_state player_state  = STOP;
volatile enum player_state decoder_state = PREPARE;
volatile enum player_state sound_state   = STOP;

uint32_t win_width, win_height;

uint8_t  *decoder_buffer;
extern int resampler_size;
extern int sample_rate;

int64_t  rewind_pos;

int64_t stream_duration;

/* Read-ahead queue limits, sized in main() from FREE RAM. On slow media (USB
 * stick, reads slowing with file position) the only thing that hides a
 * sustained read-speed shortfall is a deep buffer: the surplus accumulated
 * while reading is fast carries playback through the stretches where it is
 * slow - the same effect as pausing to let the reader catch up, but automatic.
 * So take a healthy share of free RAM instead of a token few megabytes.
 * Defaults = the old flat caps, until main() sizes them. */
static int video_queue_limit = 4*1024*1024;
static int audio_queue_limit = 4*1024*1024;

#define QUEUE_FLOOR           (4*1024*1024)   /* never below the old cap        */
#define VIDEO_QUEUE_CEILING   (256*1024*1024) /* hard upper bound               */
#define AUDIO_QUEUE_CEILING   (32*1024*1024)  /* compressed audio is small      */

/* KolibriOS syscall 18.16: size of free RAM in kilobytes. */
static uint32_t get_free_ram_kb(void)
{
    uint32_t kb;
    __asm__ __volatile__("int $0x40":"=a"(kb):"a"(18),"b"(16));
    return kb;
}

/* --- Custom AVIOContext with a large I/O buffer ---------------------------- *
 * KolibriOS file reads (syscall 70) are STATELESS: every read passes an
 * absolute byte offset and the FAT driver re-walks the cluster chain from the
 * file's first cluster to reach it - O(offset) per read - with only a tiny
 * FAT-sector cache and no read-ahead on USB. ffmpeg's default file buffer is
 * just 32 KB, so it issues dozens of these O(offset) reads per second, and the
 * cost climbs the deeper into the file you play: smooth at the start, stalling
 * from the middle on a USB stick. Reading in far larger chunks amortizes each
 * chain-walk over ~1 MB instead of 32 KB, cutting the number of positioned
 * reads (and thus the aggregate walk cost) by ~32x. */
#define AVIO_BUF_SIZE   (1024*1024)

/* Large-file (>2 GB) support. newlib's file layer is 32-bit (_off_t is a signed
 * long, ioh->offset is unsigned int), so lseek()/read() cannot address past
 * 2 GB / 4 GB. We bypass newlib entirely: keep our own 64-bit file position and
 * call KolibriOS syscall 70 (read file / get info) directly - its fileinfo
 * block carries a 64-bit offset. Real reach past 4 GB then depends only on the
 * filesystem driver (exFAT), not on the C library. */
typedef struct
{
    const char *path;
    int64_t     pos;
}file_io_t;

static file_io_t g_fio;

/* syscall 70 fileinfo block (packed): +0 subfn, +4/+8 offset64, +12 size,
 * +16 buffer, +20 zero, +21 path pointer. */
struct fs70op
{
    uint32_t    fn;
    uint32_t    off_lo;
    uint32_t    off_hi;
    uint32_t    size;
    void       *buf;
    uint8_t     zero;
    const char *path;
}__attribute__((packed));

static int64_t fp_filesize(const char *path)
{
    struct fs70op op;
    uint8_t       bdfe[560];       /* file-info block: 40-byte header (64-bit
                                    * size at +32) followed by the name */
    uint32_t      status;

    op.fn = 5;                     /* subfn 5: get file/dir info */
    op.off_lo = 0;
    op.off_hi = 0;
    op.size   = 0;
    op.buf    = bdfe;
    op.zero   = 0;
    op.path   = path;

    __asm__ __volatile__("int $0x40":"=a"(status):"a"(70),"b"(&op):"memory");
    if(status != 0)
        return -1;
    return *(int64_t*)(bdfe + 32);
}

static int fp_read_packet(void *opaque, uint8_t *buf, int buf_size)
{
    file_io_t *f = opaque;
    struct fs70op op;
    uint32_t   status, rw;

    op.fn = 0;                     /* subfn 0: read file */
    op.off_lo = (uint32_t) f->pos;
    op.off_hi = (uint32_t)(f->pos >> 32);
    op.size   = buf_size;
    op.buf    = buf;
    op.zero   = 0;
    op.path   = f->path;

    __asm__ __volatile__("int $0x40":"=a"(status),"=b"(rw):"a"(70),"b"(&op):"memory");

    if(rw > 0)
    {
        f->pos += rw;
        return (int)rw;
    }
    return AVERROR_EOF;            /* status 6 (EOF) or a read error */
}

static int64_t fp_seek(void *opaque, int64_t offset, int whence)
{
    file_io_t *f = opaque;

    whence &= ~AVSEEK_FORCE;

    if(whence == AVSEEK_SIZE)
        return fp_filesize(f->path);

    if(whence == SEEK_CUR)
        offset += f->pos;
    else if(whence == SEEK_END)
    {
        int64_t sz = fp_filesize(f->path);
        if(sz < 0)
            return -1;
        offset += sz;
    }
    /* SEEK_SET: offset is already absolute */

    f->pos = offset;
    return f->pos;
}

volatile int threads_running = DECODER_THREAD;

/* Set by the O hotkey (video.c): the file to play next. main() execs a fresh
 * instance with it as its very last step, after the sound stream, demuxer and
 * display pipeline have all been torn down. */
char g_next_file[1024];

extern char __pgmname[];             /* full path of this executable (header) */

/* File reading runs on its own thread (read_thread) so a slow, blocking read -
 * e.g. from a USB stick where each read is O(file-position) - never stalls
 * audio/video decoding. The decoder consumes the mutex-protected packet queues;
 * the reader fills them. These flags coordinate the two, and let the decoder
 * park the reader while it flushes and seeks (av_read_frame and
 * avformat_seek_file must not touch the demuxer concurrently). */
static volatile int reader_run    = 0;   /* decoder -> reader: read or idle    */
static volatile int reader_eof    = 0;   /* reader  -> decoder: end of file    */
static volatile int reader_paused = 0;   /* reader  -> decoder: idle, safe to seek */

/* Raw command-line buffer from the app header (linker symbol ___cmdline in
 * app-dynamic.lds -> C name __cmdline). Note: newlib's startup (crt1.c
 * split_cmdline) has ALREADY tokenized this buffer IN PLACE by the time main()
 * runs - it overwrites the first space of every gap with '\0' and fills argv[].
 * So __cmdline is only reliable for its FIRST char (to tell whether the
 * parameter was quoted); the file name(s) are recovered from argv[]. */
extern char __cmdline[];

int main( int argc, char *argv[])
{
    static vst_t vst;
    int i, ret;
    char *file_name, *dot;

    /* KolibriOS command-line convention:
     *   - a parameter WITHOUT quotes is a single string = one file path, taken
     *     whole (spaces and all);
     *   - a parameter that STARTS WITH a quote is one or several quoted file
     *     names.
     * With no parameter, fall back to the open dialog. */
    {
        char *raw = __cmdline;

        while(*raw == ' ' || *raw == '\t')     /* first meaningful char */
            raw++;

        if(*raw == 0 || argc < 2)
        {
            vst.input_file = get_moviefile();
            if(vst.input_file == NULL)
            {
                printf("Please provide a movie file\n");
                return -1;
            }
        }
        else if(*raw == '\"')
        {
            /* Quoted: newlib already split the quoted names into argv[] with
             * the quotes removed and the spaces inside each name preserved, so
             * argv[1] is the first file complete. fplay plays one file;
             * argv[2..argc-1] would be a playlist. */
            vst.input_file = argv[1];
        }
        else
        {
            /* Unquoted: the whole line is ONE path. newlib destructively split
             * it on spaces (a '\0' over the first space of each gap), so stitch
             * the argv tokens back together in place - every embedded '\0' from
             * argv[1] to the end of the last token becomes a space again. It
             * left any extra spaces of a gap intact, so the original spacing is
             * restored exactly. */
            char *end = argv[argc - 1] + strlen(argv[argc - 1]);
            char *s;

            for(s = argv[1]; s < end; s++)
                if(*s == 0)
                    *s = ' ';

            vst.input_file = argv[1];
        }
    }

    /* register all codecs, demux and protocols */

    av_log_set_level(AV_LOG_FATAL);

    avcodec_register_all();
    avdevice_register_all();
    av_register_all();

    /* Open with a large custom, 64-bit I/O layer (fp_*): a big buffer to
     * minimize the number of KolibriOS positioned reads (each O(offset) in the
     * FAT driver), and a 64-bit file position so files >2 GB work (newlib's
     * file layer is only 32-bit). */
    {
        unsigned char *iobuf;
        AVIOContext   *avio;

        if(fp_filesize(vst.input_file) < 0)
        {
            printf("Cannot open file %s\n\r", vst.input_file);
            return -1;
        }

        g_fio.path = vst.input_file;
        g_fio.pos  = 0;

        iobuf = av_malloc(AVIO_BUF_SIZE);
        avio  = avio_alloc_context(iobuf, AVIO_BUF_SIZE, 0,
                                   &g_fio,
                                   fp_read_packet, NULL, fp_seek);

        vst.fCtx = avformat_alloc_context();
        vst.fCtx->pb     = avio;
        vst.fCtx->flags |= AVFMT_FLAG_CUSTOM_IO;

        if( avformat_open_input(&vst.fCtx, vst.input_file, NULL, NULL) < 0)
        {
            printf("Cannot open file %s\n\r", vst.input_file);
            return -1; // Couldn't open file
        };
    }

    vst.fCtx->flags |= AVFMT_FLAG_GENPTS;

    if(avformat_find_stream_info(vst.fCtx, NULL) < 0)
    {
        printf("Cannot find streams\n\r");
        return -1;
    };

    file_name = strrchr(vst.input_file,'/')+1;
    dot = strrchr(file_name,'.');
    if(dot)
    {
        vst.input_name = malloc(dot-file_name+1);
        memcpy(vst.input_name, file_name, dot-file_name);
        vst.input_name[dot-file_name] = 0;
    }
    else vst.input_name = file_name;

    /* An unknown duration is AV_NOPTS_VALUE - a huge NEGATIVE int64, not 0
     * (e.g. mkv/webm rips muxed live carry no duration at all). Sanitize every
     * source to "0 = unknown", or the progress bar and the time display get
     * garbage like -596523:-14. */
    stream_duration = vst.fCtx->duration;
    if(stream_duration < 0)
        stream_duration = 0;

    vst.vStream = -1;
    vst.aStream = -1;

    for(i=0; i < vst.fCtx->nb_streams; i++)
    {
        if(vst.fCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO
            && vst.vStream < 0)
        {
            vst.vStream = i;
            vst.video_time_base = vst.fCtx->streams[i]->time_base;
            if(stream_duration == 0 && vst.fCtx->streams[i]->duration > 0)
               stream_duration = vst.fCtx->streams[i]->duration;
        }

        if(vst.fCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO &&
           vst.aStream < 0)
        {
            vst.aStream = i;
            if(stream_duration == 0 && vst.fCtx->streams[i]->duration > 0)
               stream_duration = vst.fCtx->streams[i]->duration;
        }
    }

    if(vst.vStream==-1)
    {
        printf("Video stream not detected\n\r");
        return -1; // Didn't find a video stream
    };

    /* Size the read-ahead queues from FREE RAM: the video queue gets 1/4 of
     * it, the audio queue 1/32 (compressed audio is tiny - 32 MB is ~20 min).
     * On slow media the deep buffer is what keeps playback smooth: while the
     * source reads fast the reader banks a surplus, and playback then rides
     * that surplus through the slow stretches (the automatic version of the
     * pause-and-let-it-buffer trick). Floors keep the old 4 MB behaviour on a
     * RAM-starved machine; ceilings stop a big machine from hoarding.
     * NB: this is a read-ahead buffer of *compressed* packets - it does NOT
     * make a stream decodable if the CPU cannot decode it in real time. */
    {
        int64_t  lim, alim;
        uint32_t free_kb = get_free_ram_kb();

        lim  = (int64_t)free_kb * 1024 / 4;            /* 1/4 of free RAM      */
        alim = (int64_t)free_kb * 1024 / 32;           /* 1/32 of free RAM     */

        if(lim  < QUEUE_FLOOR)           lim  = QUEUE_FLOOR;
        if(lim  > VIDEO_QUEUE_CEILING)   lim  = VIDEO_QUEUE_CEILING;
        if(alim < QUEUE_FLOOR)           alim = QUEUE_FLOOR;
        if(alim > AUDIO_QUEUE_CEILING)   alim = AUDIO_QUEUE_CEILING;

        video_queue_limit = (int)lim;
        audio_queue_limit = (int)alim;

        printf("read-ahead: video %d KB, audio %d KB (free RAM %u KB)\n",
               video_queue_limit/1024, audio_queue_limit/1024, free_kb);
    }


    INIT_LIST_HEAD(&vst.input_list);
    INIT_LIST_HEAD(&vst.output_list);
    mutex_init(&vst.q_video.lock);
    mutex_init(&vst.q_audio.lock);
    mutex_init(&vst.gpu_lock);
    mutex_init(&vst.decoder_lock);
    mutex_init(&vst.input_lock);
    mutex_init(&vst.output_lock);

    vst.vCtx = vst.fCtx->streams[vst.vStream]->codec;
    /* aStream stays -1 for a video-only file (e.g. a phone clip with no audio
     * track); streams[-1] would fault. Leave aCtx NULL and skip all audio. */
    vst.aCtx = (vst.aStream >= 0)
             ? vst.fCtx->streams[vst.aStream]->codec
             : NULL;

//    __asm__ __volatile__("int3");

    if(init_video_decoder(&vst) != 0 )
        return -1;

    if(vst.aCtx)
    {
        vst.aCtx->request_channel_layout = AV_CH_LAYOUT_STEREO;
        vst.aCodec = avcodec_find_decoder(vst.aCtx->codec_id);
    }
    if(vst.aCtx && vst.aCodec)
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

            vst.snd_format = test_wav(&whdr);

            if( init_audio(&vst) )
            {
                decoder_buffer = (uint8_t*)av_mallocz(192000*2+64);
                if( decoder_buffer != NULL )
                {
                    mutex_init(&astream.lock);
                    astream.count  = 0;
                    astream.buffer = (char *)av_mallocz(192000*3);
                    if( astream.buffer != NULL )
                        vst.has_sound = 1;
                    else
                        av_free(decoder_buffer);
                }
                if( vst.has_sound == 0)
                {
                        printf("Not enough memory for audio buffers\n");
                }
            }
        }
        else printf("Cannot open audio codec\n\r");
    }
    else if(vst.aCtx == NULL)
        printf("No audio stream\n");
    else printf("Unsupported audio codec!\n");

    mutex_lock(&vst.decoder_lock);
    create_thread(video_thread, &vst, 1024*1024);
    if(mutex_lock_timeout(&vst.decoder_lock, 3000) == 0)
        return -1;

    create_thread(read_thread, &vst, 1024*1024);

    decoder(&vst);


//__asm__ __volatile__("int3");

    /* wait for the reader too, so it is out of av_read_frame before we close
     * the codecs / free the format context below */
    while( threads_running &
           (AUDIO_THREAD | VIDEO_THREAD | READER_THREAD))
           delay(1);

    if(astream.lock.handle)
        mutex_destroy(&astream.lock);

    vst.decoder->fini(&vst);
    avcodec_close(vst.vCtx);

    mutex_destroy(&vst.q_video.lock);
    mutex_destroy(&vst.q_audio.lock);
    mutex_destroy(&vst.decoder_lock);

    /* O hotkey: relaunch the player with the newly chosen file, now that
     * everything (sound stream, DRM, demuxer, window) is released - the new
     * instance starts as cleanly as one launched from the file manager. */
    if(g_next_file[0] != 0)
    {
        struct
        {
            uint32_t    fn;          /* +0  = 7 (execute)      */
            uint32_t    flags;       /* +4  0 = no debug       */
            const char *args;        /* +8  command line       */
            uint32_t    r12;         /* +12 reserved           */
            uint32_t    r16;         /* +16 reserved           */
            uint8_t     zero;        /* +20                    */
            const char *path;        /* +21 program path       */
        }__attribute__((packed)) op;
        int status;

        /* Pass the path unquoted - a single string, which main() takes whole
         * (the "unquoted = one file path" convention), exactly like a file
         * manager launching one file. */
        op.fn    = 7;
        op.flags = 0;
        op.args  = g_next_file;
        op.r12   = 0;
        op.r16   = 0;
        op.zero  = 0;
        op.path  = __pgmname;

        __asm__ __volatile__("int $0x40":"=a"(status):"a"(70),"b"(&op):"memory");
        if(status < 0)
            printf("restart failed: %d\n", status);
    }

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
                  (vst->has_sound != 0) )
        {
            put_packet(&vst->q_audio, &packet);
            if(vst->audio_timer_valid == 0 &&
               packet.pts != AV_NOPTS_VALUE )
            {
                vst->audio_timer_base = get_audio_base(vst) * packet.pts;
                vst->audio_timer_valid = 1;
            };
        }
        else av_free_packet(&packet);
    }
    else if (err != AVERROR_EOF)
        printf("av_read_frame: error %x\n", err);

    return err;
}



/* Whether the demuxer should read another packet. The read is throttled to
 * real time by the video queue filling up, but only q_video was ever bounded.
 * q_audio has no consumer-side backpressure on the FEED (decode_audio refuses
 * to DRAIN once astream is full, but load_frame keeps pushing), so whenever the
 * video byte-rate falls below real time - a slow software-blitter decode, a
 * low-bitrate stretch, or a video track that ends before the audio - q_video
 * never reaches its cap, reading never stops, and q_audio grows without bound.
 * On KolibriOS that steadily raises heap pressure and every av_malloc in the
 * decode path, so fps sags further into a long file. Cap BOTH queues.
 * video_queue_limit is bitrate-sized in main(); the audio cap stays flat. */
static int want_more_data(vst_t* vst)
{
    return (vst->q_video.size < video_queue_limit) &&
           (vst->q_audio.size < audio_queue_limit);
}

/* Dedicated file-reading thread. Fills the packet queues whenever the decoder
 * has enabled it (reader_run) and the queues have room. A blocking read here
 * cannot stall decode/present, which run on other threads.
 *
 * PACING: the reader must NOT run flat out. Every positioned read costs a
 * long in-kernel FAT chain walk (O(file position)) plus the data copy, and
 * with a deep RAM cache the cap stays unreached for minutes - an unthrottled
 * reader then hogs the CPU continuously, starving the decoder thread (which
 * also refills the audio ring) and the audio thread: the sound stutters MORE
 * with a bigger cache, not less. So the reader always yield()s between
 * packets (anyone who wants the CPU gets it first) and takes a 10 ms breath
 * at regular intervals - rare while the reserve is thin (catch up quickly),
 * frequent once a comfortable reserve exists (cruise). */
#define READER_LOW_WATERMARK  (16*1024*1024)
#define READER_STARVED        (2*1024*1024)

int read_thread(void *param)
{
    vst_t *vst = param;
    int    since_rest = 0;

    __sync_or_and_fetch(&threads_running, READER_THREAD);

    while( player_state != CLOSED )
    {
        int rest_every;

        if( !reader_run )
        {
            reader_paused = 1;          /* acknowledge: idle, safe to seek */
            delay(1);
            continue;
        }
        reader_paused = 0;

        if( reader_eof || !want_more_data(vst) )
        {
            delay(1);                   /* at EOF or queues full: back off  */
            continue;
        }

        if( load_frame(vst) != 0 )      /* AVERROR_EOF or a read error      */
        {
            reader_eof = 1;
            continue;
        }

        /* Near-empty queue = the decoder is about to run dry (slow medium:
         * USB stick, network): read flat out, yield()ing so decode/audio can
         * always preempt, but skip the 10 ms breaths - they only slow the
         * refill down exactly when every packet matters. The breaths resume
         * once a couple of seconds of reserve exist. */
        if(vst->q_video.size < READER_STARVED)
        {
            since_rest = 0;
            yield();
        }
        else
        {
            rest_every = (vst->q_video.size < READER_LOW_WATERMARK) ? 16 : 4;
            if(++since_rest >= rest_every)
            {
                since_rest = 0;
                delay(1);               /* guaranteed CPU for decode/audio  */
            }
            else
                yield();
        }
    }

    /* Signal "parked" on exit as well, so a reader_pause() racing shutdown
     * (window closed mid-seek) unblocks and knows the reader is out of
     * av_read_frame - otherwise it would either hang or, with a CLOSED escape,
     * let the decoder seek fCtx concurrently with a live read. */
    reader_paused = 1;
    __sync_and_and_fetch(&threads_running, ~READER_THREAD);
    return 0;
}

/* Park the reader and wait until it is provably idle (out of av_read_frame),
 * so the decoder can flush the queues and seek the demuxer without racing it.
 * The reader sets reader_paused when it parks (reader_run==0) AND when it exits
 * on CLOSED, so this always terminates - bounded by one in-flight read. */
static void reader_pause(void)
{
    reader_run = 0;
    while( reader_paused == 0 )
        delay(1);
}

/* Re-arm the reader from the file's current (post-seek) position. */
static void reader_resume(void)
{
    reader_eof    = 0;
    reader_paused = 0;
    reader_run    = 1;
}


static void flush_all(vst_t* vst)
{
    AVPacket  packet;

    avcodec_flush_buffers(vst->vCtx);
    if(vst->aCtx)                    /* NULL for a video-only file */
        avcodec_flush_buffers(vst->aCtx);
    while( get_packet(&vst->q_video, &packet) != 0)
        av_free_packet(&packet);

    while( get_packet(&vst->q_audio, &packet)!= 0)
        av_free_packet(&packet);

    flush_video(vst);

    /* Force the reader to re-anchor the sound clock from the FIRST audio
     * packet of the new position (load_frame updates audio_timer_base only
     * while audio_timer_valid==0). The audio thread clears this flag in its
     * PLAY_2_STOP - but only when it was actively playing, and not at all
     * when wait_sound_stopped() timed out past a wedged audio thread. A stale
     * flag leaves audio_timer_base at the PRE-seek position, SetTimeBase then
     * anchors the master clock minutes away from the new frames, and the
     * video free-runs unpaced (the post-seek 45-fps desync). The reader is
     * parked here (reader_pause), so this write cannot race load_frame. */
    vst->audio_timer_valid = 0;

    /* Zero the PCM staging buffer under its lock. This USED to rely on the
     * sound_state==STOP handshake proving the audio thread was quiescent, but
     * wait_sound_stopped() may now give up on a wedged audio thread (infinite
     * GetNotify) and let us proceed WITHOUT that guarantee. Every other
     * astream.count accessor (decode_audio + the audio thread) already takes
     * astream.lock, so taking it here makes the reset mutually exclusive with
     * them - the race is closed, not merely moved. Safe for video-only files
     * too: has_sound==0 means no audio thread, so the lock is uncontended and
     * mutex_lock takes the fast path without touching the (uninit) handle. */
    mutex_lock(&astream.lock);
    astream.count = 0;
    mutex_unlock(&astream.lock);
};

/* Wait for the audio thread to acknowledge a stop (sound_state==STOP) before we
 * flush astream and seek. Two failure modes, two defences:
 *
 *  1. A rapid second seek can make the audio thread overwrite our PLAY_2_STOP
 *     with PLAY on its restart fall-through (audio.c), losing the command. So
 *     every ~100 ms we re-assert PLAY_2_STOP; a live audio thread acknowledges
 *     it on its next loop iteration.
 *
 *  2. The audio thread can wedge PERMANENTLY in its infinite GetNotify (syscall
 *     68.14): a seek's ResetBuffer(SND_RESET_ALL) drops the stream from the
 *     mixer playlist, and on real hardware the sound-card DMA can then underrun
 *     and stop raising completion events, which the engine never re-arms (a
 *     later PlayBuffer does not re-issue dev_play). A thread blocked in
 *     GetNotify never re-reads sound_state, so re-assertion is powerless and we
 *     would spin here forever -> the decoder never leaves REWIND and the video
 *     freezes. So bound the wait: after ~500 ms with no STOP, force the state to
 *     STOP and proceed. This is race-free because flush_all() now zeroes astream
 *     under astream.lock, and leaving STOP (not PLAY_2_STOP) means a late-waking
 *     audio thread parks in case STOP instead of clobbering the PREPARE the
 *     decoder is about to issue. Video always recovers; audio resumes on the
 *     next PREPARE if the ring is still alive (the wedge was transient). */
static void wait_sound_stopped(vst_t* vst)
{
    int spin = 0, total = 0;

    while(vst->has_sound && sound_state != STOP && player_state != CLOSED)
    {
        delay(1);
        if(++spin >= 10)
        {
            sound_state = PLAY_2_STOP;
            spin = 0;
        }
        if(++total >= 50)
        {
            sound_state = STOP;
            break;
        }
    }
}

void decoder(vst_t* vst)
{
    int       ret, vret, aret;
    int       guard;
    int64_t   min_pos, max_pos;

//    av_log_set_level(AV_LOG_DEBUG);

    while( player_state != CLOSED )
    {
        switch(decoder_state)
        {
            case PREPARE:
                reader_resume();       /* start filling the queues */

                /* Prime the audio buffer, then start playback. The reader fills
                 * the queues asynchronously, so wait when there is nothing to
                 * decode yet. But do NOT block here forever: during PREPARE the
                 * output frames are not recycled (that only happens under PLAY),
                 * so q_video can fill to its cap and stop the reader before it
                 * reaches the audio packets - priming would then never complete.
                 * The stall guard bounds the wait (~1 s of idle time) and drops
                 * into PLAY, where frames recycle, q_video drains and the reader
                 * resumes. Also bail immediately on shutdown. */
                guard = 0;
                do
                {
                    vret = decode_video(vst);
                    aret = decode_audio(vst->aCtx, &vst->q_audio);
                    if( vret <= 0 && aret <= 0 )
                    {
                        delay(1);
                        guard++;
                    }
                }while(player_state != CLOSED &&
                       astream.count < resampler_size*2 &&
                       !(reader_eof && vst->q_audio.count == 0) &&
                       guard < 100);

                if(player_state == CLOSED)
                    continue;

                sound_state   = PREPARE;
                decoder_state = PLAY;
                player_state  = PLAY;

            case PLAY:
                vret = decode_video(vst);

                /* AUDIO FIRST: drain the audio queue until the PCM ring is
                 * full (decode_audio self-limits via astream.count) instead of
                 * rationing one packet per video frame. When the video codec
                 * is slower than real time (e.g. VP9 on an old CPU takes
                 * 100+ ms a frame), one-audio-packet-per-iteration starves the
                 * sound to a fraction of real time and it stutters even
                 * though audio decode itself is nearly free. Sound must never
                 * depend on how fast the video decodes. */
                do
                {
                    aret = decode_audio(vst->aCtx, &vst->q_audio);
                }while(aret == 1);

                ret  = vret | aret;

                if( reader_eof && !ret &&
                    vst->q_video.count == 0 && vst->q_audio.count == 0)
                {
                    decoder_state = STOP;
                    continue;
                };

                if( vret == 1 )                  /* decoded a video frame */
                {
                    yield();
                    continue;
                }

                delay(1);        /* queues empty (reader catching up) or full */
                continue;

            case STOP:
                delay(1);
                continue;


            case PLAY_2_STOP:
                wait_sound_stopped(vst);
                if(player_state == CLOSED)
                    continue;

                reader_pause();        /* stop the reader before touching fCtx */
                flush_all(vst);

                if (vst->fCtx->start_time != AV_NOPTS_VALUE)
                    rewind_pos = vst->fCtx->start_time;
                else
                    rewind_pos = 0;

                ret = avformat_seek_file(vst->fCtx, -1, INT64_MIN,
                                         rewind_pos, INT64_MAX, 0);

                reader_eof = 0;        /* reader stays parked until next PREPARE */
                decoder_state = STOP;
                continue;

            case REWIND:
                wait_sound_stopped(vst);
                if(player_state == CLOSED)
                    continue;

                reader_pause();        /* stop the reader before touching fCtx */
                flush_all(vst);

                if(rewind_pos < 0)
                {
                    rewind_pos = -rewind_pos;
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
                reader_eof = 0;
                decoder_state = PREPARE;   /* PREPARE re-arms the reader */
                continue;
        }
    };
};

