
#define BLACK_MAGIC_SOUND
#define BLACK_MAGIC_VIDEO

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

extern astream_t astream;
extern AVRational video_time_base;

int init_audio(int format);
int init_video(AVCodecContext *ctx);
int decode_video(AVCodecContext  *ctx, AVPacket *pkt);
double get_master_clock();


int create_thread(void (*proc)(void *param), void *param, int stack_size);

void spinlock_lock(volatile uint32_t *val);

static inline void spinlock_unlock(volatile uint32_t *val)
{
    *val = 0;
}

static inline void GetNotify(void *event)
{
    __asm__ __volatile__ (
    "int $0x40"
    ::"a"(68),"b"(14),"c"(event));
}

static inline uint32_t check_os_event()
{
    uint32_t val;
    __asm__ __volatile__(
    "int $0x40"
    :"=a"(val)
    :"a"(11));
    return val;
};

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
    ::"a"(5), "b"(time));
};
