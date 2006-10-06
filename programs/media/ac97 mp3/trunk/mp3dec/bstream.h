typedef unsigned char byte;
typedef unsigned int uint;

typedef void (*SBT_PROC) (float *sample, void *pcm, int n);
typedef void (*XFORM_PROC) (void *pcm, int igr);

void bitget_init(unsigned char *buf);
void bitget_init_end(unsigned char *buf_end);
int bitget_overrun();
int bitget_bits_used();
void bitget_check(int n);
int bitget(int n);
void bitget_skip(int n);
//int bitget_1bit();

int bitget_lb(int n);
int bitget2(int n);
void bitget_purge(int n);
void mac_bitget_check(int n);
int mac_bitget(int n);
int mac_bitget_1bit();
int mac_bitget2(int n);
void mac_bitget_purge(int n);
