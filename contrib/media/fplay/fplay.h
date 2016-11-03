
#include <libsync.h>
#include <pixlib3.h>
#include "list.h"

#define BLACK_MAGIC_SOUND
#define BLACK_MAGIC_VIDEO

typedef unsigned int color_t;
typedef unsigned int count_t;

typedef struct render  render_t;
typedef struct vstate vst_t;

#define HAS_LEFT        (1<<0)
#define HAS_TOP         (1<<1)
#define HAS_RIGHT       (1<<2)
#define HAS_BOTTOM      (1<<3)

typedef struct
{
    struct list_head list;
    enum AVPixelFormat format;
    AVPicture     picture;
    planar_t*     planar;
    int           is_hw_pic;
    int           index;
    double        pts;
    double        pkt_pts;
    volatile int  ready;
}vframe_t;

struct render
{
    vst_t     *vst;
    uint32_t   caps;
    uint32_t   ctx_width;
    uint32_t   ctx_height;
    uint32_t   win_width;
    uint32_t   win_height;

    rect_t     rc_client;
    rect_t     rcvideo;
    rect_t     rcleft;
    rect_t     rctop;
    rect_t     rcright;
    rect_t     rcbottom;

    uint32_t   layout;
    bitmap_t  *bitmap[4];
    bitmap_t  *last_bitmap;

    uint32_t   ctx_format;
    int        target;

    window_t   *win;
    enum{
      EMPTY, INIT }state;
    enum win_state win_state;

    void (*draw)(render_t *render, vframe_t *vframe);
};

enum player_state
{
    CLOSED = 0,
    PREPARE,
    STOP,
    PAUSE,
    PLAY,
    REWIND,
    PLAY_2_STOP,
    PLAY_2_PAUSE,
    PAUSE_2_PLAY,
    REWIND_2_PLAY,
};

#define ID_PLAY             100
#define ID_STOP             101
#define ID_PROGRESS         102
#define ID_VOL_LEVEL        103
#define ID_VOL_CTRL         104

typedef struct
{
    mutex_t lock;
    char    *buffer;
    int     count;
}astream_t;

typedef struct
{
  unsigned int  code;
  unsigned int  sender;
  unsigned int  stream;
  unsigned int  offset;
  unsigned int  size;
  unsigned int  unused[2];
}SND_EVENT;


typedef struct {
    AVPacketList *first_pkt;
    AVPacketList *last_pkt;
    int size;
    int count;
    mutex_t       lock;
} queue_t;

int put_packet(queue_t *q, AVPacket *pkt);
int get_packet(queue_t *q, AVPacket *pkt);

struct decoder
{
    const char     *name;
    enum AVCodecID  codec_id;
    int             profile;
    enum AVPixelFormat pix_fmt;
    int             width;
    int             height;
    AVFrame        *Frame;
    vframe_t       *active_frame;
    vframe_t        vframes[16];
    int             nframes;
    int             is_hw:1;
    int             has_surfaces:1;
    int             frame_reorder:1;
    void          (*fini)(vst_t *vst);
};

typedef struct decoder* decoder_init_fn(vst_t *vst);
struct decoder* init_va_decoder(vst_t *vst);

struct vstate
{
    AVFormatContext *fCtx;              /* format context           */
    AVCodecContext  *vCtx;              /* video decoder context    */
    AVCodecContext  *aCtx;              /* audio decoder context    */
    AVCodec         *vCodec;            /* video codec              */
    AVCodec         *aCodec;            /* audio codec              */
    char            *input_file;
    char            *input_name;
    int             vStream;            /* video stream index       */
    int             aStream;            /* audio stream index       */
    AVRational      video_time_base;
    double          audio_timer_base;

    queue_t         q_video;            /* video packets queue      */
    queue_t         q_audio;            /* audio packets queue      */

    mutex_t         gpu_lock;           /* gpu access lock. libdrm not yet thread safe :( */
    mutex_t         decoder_lock;

    mutex_t         input_lock;
    mutex_t         output_lock;
    struct list_head input_list;
    struct list_head output_list;
    struct list_head destructor_list;

    struct decoder *decoder;
    int             snd_format;
    volatile int    frames_count;
    unsigned int    has_sound:1;
    unsigned int    audio_timer_valid:1;
    unsigned int    blit_bitmap:1;      /* hardware RGBA blitter    */
    unsigned int    blit_texture:1;     /* hardware RGBA blit and scale */
    unsigned int    blit_planar:1;      /* hardbare YUV blit and scale */
};


#define DECODER_THREAD  1
#define AUDIO_THREAD    2
#define VIDEO_THREAD    4

extern volatile int threads_running;
extern astream_t astream;

render_t *create_render(vst_t *vst, window_t *win, uint32_t flags);
void destroy_render(render_t *render);
int init_render(render_t *render, int width, int height);
void render_adjust_size(render_t *render, window_t *win);
void render_set_size(render_t *render, int width, int height);
void render_draw_client(render_t *render);

int fplay_init_context(vst_t *vst);

int init_audio(vst_t* vst);
int audio_thread(void *param);
void set_audio_volume(int left, int right);

int video_thread(void *param);
void flush_video(vst_t* vst);

int init_video_decoder(vst_t *vst);
void fini_video_decoder(vst_t *vst);
void decoder(vst_t *vst);
int decode_video(vst_t* vst);
int decode_audio(AVCodecContext  *ctx, queue_t *qa);

double get_master_clock(void);

static inline void GetNotify(void *event)
{
    __asm__ __volatile__ (
    "int $0x40"
    ::"a"(68),"b"(14),"c"(event));
}

static inline double get_audio_base(vst_t* vst)
{
    return (double)av_q2d(vst->fCtx->streams[vst->aStream]->time_base)*1000;
};

struct decoder* va_init_decoder(vst_t *vst);
void va_create_planar(vst_t *vst, vframe_t *vframe);

int init_fontlib();
char *get_moviefile();

#define ENTER()   printf("enter %s\n",__FUNCTION__)
#define LEAVE()   printf("leave %s\n",__FUNCTION__)
#define FAIL()    printf("fail %s\n",__FUNCTION__)


