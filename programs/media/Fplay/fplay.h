
#define BLACK_MAGIC_SOUND
#define BLACK_MAGIC_VIDEO

typedef unsigned int color_t;
typedef unsigned int count_t;

typedef struct
{
    uint32_t    width;
    uint32_t    height;
    uint32_t    pitch;
    uint32_t    handle;
    uint8_t    *data;
}bitmap_t;

typedef struct render  render_t;

#define HAS_LEFT        (1<<0)
#define HAS_TOP         (1<<1)
#define HAS_RIGHT       (1<<2)
#define HAS_BOTTOM      (1<<3)

struct render
{
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
    bitmap_t   bitmap[4];
    bitmap_t  *last_bitmap;

    uint32_t   ctx_format;
    int        target;

    window_t   *win;
    enum{
      EMPTY, INIT }state;
    enum win_state win_state;

    void (*draw)(render_t *render, AVPicture *picture);
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
    volatile uint32_t   lock;
    char               *buffer;
    volatile uint32_t   count;
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

typedef struct
{
  unsigned      handle;
  unsigned      io_code;
  void          *input;
  int           inp_size;
  void          *output;
  int           out_size;
}ioctl_t;

typedef struct {
    AVPacketList *first_pkt;
    AVPacketList *last_pkt;
    int size;
    int count;
    volatile uint32_t lock;
} queue_t;

int put_packet(queue_t *q, AVPacket *pkt);
int get_packet(queue_t *q, AVPacket *pkt);


extern astream_t astream;
extern AVRational video_time_base;

render_t *create_render(uint32_t width, uint32_t height,
                        uint32_t ctx_format, uint32_t flags);

int init_render(render_t *render, int width, int height);
void render_adjust_size(render_t *render, window_t *win);
int render_set_size(render_t *render, int width, int height);
void render_draw_client(render_t *render);


int init_audio(int format);
int audio_thread(void *param);
void set_audio_volume(int left, int right);

int init_video(AVCodecContext *ctx);
int video_thread(void *param);

int decode_video(AVCodecContext  *ctx, queue_t *qv);
int decode_audio(AVCodecContext  *ctx, queue_t *qa);

double get_master_clock(void);


int create_thread(int (*proc)(void *param), void *param, int stack_size);

void mutex_lock(volatile uint32_t *val);

static inline void mutex_unlock(volatile uint32_t *val)
{
    *val = 0;
}

static inline void GetNotify(void *event)
{
    __asm__ __volatile__ (
    "int $0x40"
    ::"a"(68),"b"(14),"c"(event));
}

static inline uint32_t get_os_button()
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(17));
    return val>>8;
};

static inline void yield(void)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(68), "b"(1));
};

static inline void delay(uint32_t time)
{
    __asm__ __volatile__(
    "int $0x40"
    ::"a"(5), "b"(time)
    :"memory");
};




#define HW_BIT_BLIT         (1<<0)      /* BGRX blitter             */
#define HW_TEX_BLIT         (1<<1)      /* stretch blit             */
#define HW_VID_BLIT         (1<<2)      /* planar and packed video  */

uint32_t InitPixlib(uint32_t flags);

int create_bitmap(bitmap_t *bitmap);
int lock_bitmap(bitmap_t *bitmap);
int resize_bitmap(bitmap_t *bitmap);
int blit_bitmap(bitmap_t *bitmap, int dst_x, int dst_y,
                int w, int h);

int init_fontlib();
