//
//   This file is part of the AC97 mp3 player.
//   (C) copyright Serge 2006
//   email: infinity_sound@mail.ru
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.


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

#define ST_DONE 0x0
#define ST_PLAY 0x1
#define ST_EXIT 0x2
#define ST_STOP 0x4

typedef struct
{  DWORD riff_id;
    DWORD riff_size;
    DWORD riff_format;

    DWORD fmt_id;
    DWORD fmt_size;

    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    DWORD data_id;
    DWORD data_size;
} WAVEHEADER;

DWORD test_wav(WAVEHEADER *hdr);
DWORD test_mp3(char *buf);

//void (*snd_play)();
void wave_out(char* buff);

void play_wave();
void play_mp3();

void snd_stop();
