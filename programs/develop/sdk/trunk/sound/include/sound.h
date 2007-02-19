
#ifndef _SOUND_H_
#define _SOUND_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define SOUND_VERSION  5

#define PCM_ALL       0
#define PCM_STATIC    0x80000000
#define PCM_2_16_48   1
#define PCM_1_16_48   2
#define PCM_2_16_44   3
#define PCM_1_16_44   4
#define PCM_2_16_32   5
#define PCM_1_16_32   6
#define PCM_2_16_24   7
#define PCM_1_16_24   8
#define PCM_2_16_22   9
#define PCM_1_16_22  10
#define PCM_2_16_16  11
#define PCM_1_16_16  12
#define PCM_2_16_12  13
#define PCM_1_16_12  14
#define PCM_2_16_11  15
#define PCM_1_16_11  16
#define PCM_2_16_8   17
#define PCM_1_16_8   18
#define PCM_2_8_48   19
#define PCM_1_8_48   20
#define PCM_2_8_44   21
#define PCM_1_8_44   22
#define PCM_2_8_32   23
#define PCM_1_8_32   24
#define PCM_2_8_24   25
#define PCM_1_8_24   26
#define PCM_2_8_22   27
#define PCM_1_8_22   28
#define PCM_2_8_16   29
#define PCM_1_8_16   30
#define PCM_2_8_12   31
#define PCM_1_8_12   32
#define PCM_2_8_11   33
#define PCM_1_8_11   34
#define PCM_2_8_8    35
#define PCM_1_8_8    36

#define SRV_GETVERSION      0
#define SND_CREATE_BUFF     1
#define SND_DESTROY_BUFF    2
#define SND_SETFORMAT       3
#define SND_RESET           4
#define SND_SETPOS          5
#define SND_SETBUFF         6
#define SND_SETVOLUME       7
#define SND_GETVOLUME       8
#define SND_OUT             9
#define SND_PLAY            10
#define SND_STOP            11

typedef unsigned int SNDBUF;

int _stdcall  InitSound();
SNDBUF _stdcall  CreateBuffer(unsigned int format,int size);
int _stdcall  DestroyBuffer(SNDBUF hBuff);
int _stdcall  SetBuffer(SNDBUF hBuff,void* buff,
                        int offs, int size);
int _stdcall  WaveOut(SNDBUF hBuff,void *buff, int size);
                               
int _stdcall  PlayBuffer(SNDBUF hBuff);
int _stdcall  StopBuffer(SNDBUF hBuff);

int _stdcall  GetMasterVol(int* vol);
int _stdcall  SetMasterVol(int vol);

#ifdef __cplusplus
extern "C"
}
#endif

#endif //_SOUND_H_