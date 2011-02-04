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


#define ST_DONE  0x0
#define ST_PLAY  0x1
#define ST_EXIT  0x2
#define ST_STOP  0x4
#define ST_TRACK 0x5


DWORD test_mp3(char *buf);
DWORD test_m3u(char *buf); //Asper+

//void (*snd_play)();
void wave_out(char* buff);

void play_wave();
void play_mp3();

void snd_stop();
