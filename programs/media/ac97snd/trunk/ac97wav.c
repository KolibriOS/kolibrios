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

#include "kolibri.h"
#include "stdio.h"
#include "string.h"
#include "ac97wav.h"
#include "mp3dec/mp3dec.h"

void thread_proc();
void touch(char *buf, int size);

extern char *__argv;

//extern char __path;

/*****      for debug output only
char formats[37][12] =
{ "PCM_ALL",
  "PCM_2_16_48","PCM_1_16_48","PCM_2_16_44","PCM_1_16_44",
  "PCM_2_16_32","PCM_1_16_32","PCM_2_16_24","PCM_1_16_24",
  "PCM_2_16_22","PCM_1_16_22","PCM_2_16_16","PCM_1_16_16",
  "PCM_2_16_12","PCM_1_16_12","PCM_2_16_11","PCM_1_16_11",
  "PCM_2_16_8","PCM_1_16_8","PCM_2_8_48","PCM_1_8_48",
  "PCM_2_8_44","PCM_1_8_44","PCM_2_8_32","PCM_1_8_32",
  "PCM_2_8_24","PCM_1_8_24","PCM_2_8_22","PCM_1_8_22",
  "PCM_2_8_16","PCM_1_8_16","PCM_2_8_12","PCM_1_8_12",
  "PCM_2_8_11","PCM_1_8_11","PCM_2_8_8","PCM_1_8_8"
};
*******/

DWORD hDrv;
DWORD hSound;
DWORD hBuff;
DWORD event[2];

CTRL_INFO info;

FILEINFO   fileinfo;

int m_vol;
DWORD status;
DWORD offset;
DWORD first_sync;

char *testbuff;
char *outbuf;
char *inpbuf;
int inpsize;
int outsize;

char srv_name[] = "INFINITY";
char srv_intel[] = "SOUND";
char header[] = "AC97 MP3 player";
char buttons_text[]=" Play    Stop     <<      >>     Vol-    Vol+";

MPEG_DECODE_INFO mpginfo;
MPEG_DECODE_PARAM param;

void (*snd_play)();

void draw_window()
{
   BeginDraw();

   DrawWindow(100,100,299,72,0x404040,3,0,0,0);

   make_button(7,24,45,13, 0x10|BT_NORMAL,0x808080);
   make_button(56,24,45,13, 0x11|BT_NORMAL,0x808080);
   make_button(104,24,45,13, 0x12|BT_NORMAL,0x808080);
   make_button(152,24,45,13, 0x13|BT_NORMAL,0x808080);
   make_button(200,24,45,13, 0x14|BT_NORMAL,0x808080);
   make_button(248,24,45,13, 0x15|BT_NORMAL,0x808080);

   make_button(7,41,286,11, 0x30|BT_HIDE|BT_NOFRAME,0x404040);
   draw_bar(7,41,286,11,0x404040);

   draw_bar(7,55,286,11,0x404040);
   write_text(12,58,0x004000|FONT0, __argv, strlen(__argv));
   write_text(11,57,0x00FF20|FONT0, __argv, strlen(__argv));

   write_text(8,8,0xFFFFFF|FONT0, header, strlen(header));
   write_text(12,28,0x404040|FONT0,buttons_text,strlen(buttons_text));
   write_text(11,27,0xA0FFA0|FONT0,buttons_text,strlen(buttons_text));

   EndDraw();
};

void draw_progress_bar()
{  DWORD x;
   x = 286.0f * (float)offset/(float)fileinfo.size;
   if(x==0) return;
   draw_bar(7,41,x,11,0xA0A0A0);
   draw_bar(x+7,41,286-x,11,0x404040);
};

void debug_out_str(char* str)
{
  while (*str != 0)
  {
    debug_out(*str);
    str++;
  }
}

int main()      //int argc, char *argv[])
{ DWORD fmt;
   char *thread_stack;
   DWORD r_bytes;
   int retval;

   InitHeap(1024*1024);
   if(get_fileinfo(__argv, &fileinfo)==FILE_NOT_FOUND)
      return 0;

   if((hDrv=GetService(srv_intel))==0)
     return 0;

   if ((hSound=GetService(srv_name))==0)
     return 0;

   GetDevInfo(hDrv, &info);

   m_vol = GetMasterVol(hDrv,&m_vol);
   if (m_vol > 85)
   { m_vol = 85;
      SetMasterVol(hDrv,m_vol);
   };

   _asm {fninit};
   mp3DecodeInit();
      
   testbuff = UserAlloc(4096); 
   get_fileinfo(__argv, &fileinfo);
   offset = 0;
   retval=read_file (__argv,testbuff,0,2048,&r_bytes);
   if (retval) return 0; 
   
   fmt = test_wav((WAVEHEADER*)testbuff);
   if (fmt != 0)
   {
     snd_play = &play_wave;
     outbuf = UserAlloc(32*1024);
     touch(outbuf, 32768);
     offset = 44;
   }   
   else  
   {  fmt = test_mp3(testbuff);
       if(fmt ==0) return 0;
       snd_play = &play_mp3;
       
       inpsize = mpginfo.maxInputSize*30;
       inpbuf =  UserAlloc(inpsize);
       touch(inpbuf, inpsize);
       outsize = mpginfo.outputSize*30+0x10000;
       outbuf = UserAlloc(outsize);
       touch(outbuf, outsize);
       first_sync = offset;
   };

   status = ST_PLAY;
   
   hBuff = CreateBuffer(hSound,fmt);
   if (hBuff == 0) return 0;
   thread_stack = UserAlloc(4096);
   thread_stack+=4092;

   CreateThread(thread_proc, thread_stack);

   while(1)
   {  delay(10);
      switch(status)
      {  case ST_PLAY:
           snd_play();
           continue;

         case ST_STOP:
           StopBuffer(hSound, hBuff);
           status = ST_DONE;
           continue;

         case ST_EXIT:
           StopBuffer(hSound, hBuff);
           DestroyBuffer(hSound, hBuff);
           return 0;
      };
   };
   return 0;
};

void touch(char *buf, int size)
{ int i;
    for ( i = 0;i < size; i+=4096)
      buf[i] = 0; 
};

DWORD test_mp3(char *buf)
{  int retval;
    int sync;
    WAVEHEADER whdr; 
    DWORD r_bytes=2048; 
     
    for (;;)
    {
    	  if (!mp3FindSync(buf, 2048, &sync))
     	    offset+= 2048;
        else break;
        
        if (offset >= fileinfo.size || offset >= 102400)
            return 0;

        retval = read_file (__argv,buf,offset,2048,&r_bytes);
    	  if(retval != 0)  return 0;
	  };
    offset+=sync;
    retval = read_file (__argv,buf,offset,2048,&r_bytes);
 	  if(retval != 0) return 0;
 	  
  	mp3GetDecodeInfo(buf, r_bytes, &mpginfo, 1);
    whdr.riff_id = 0x46464952;
    whdr.riff_format = 0x45564157;
    whdr.wFormatTag = 0x01;
    whdr.nSamplesPerSec = mpginfo.frequency;
    whdr.nChannels = mpginfo.channels;
    whdr.wBitsPerSample = mpginfo.bitsPerSample;
    
    return test_wav(&whdr);
};
void wave_out(char* buff)
{ DWORD ev[2];

   GetNotify(&ev[0]);
   SetBuffer(hSound,hBuff,buff,ev[1],0x8000);
}

void play_mp3()
{  int retval; 
    DWORD r_bytes;
    char *inpPtr;
    char *outPtr;
    int inpBytes;
    int totalout;
    
    offset = first_sync;
    
    retval = read_file (__argv,inpbuf,offset,inpsize,&r_bytes);
 	  if(retval != 0)
 	  {  status =  ST_STOP;
 	      return ;
    }; 
    offset+=inpsize;

    mp3DecodeStart(inpbuf, inpsize);
  
    inpPtr = inpbuf+mpginfo.skipSize;
    inpBytes = inpsize-mpginfo.skipSize;
    outPtr = outbuf;
    totalout=0;
    
    memset(outbuf,0,0x10000); 
    SetBuffer(hSound,hBuff,outbuf,0,0x10000);
    PlayBuffer(hSound, hBuff);

    _asm { fninit }
    
    while(1)
    { if(status!=ST_PLAY)
             break;
  
      for(;;)
      { param.inputBuf	= inpPtr;
	       param.inputSize	= inpBytes;
     	   param.outputBuf	= outPtr;

    	   if(!mp3DecodeFrame(&param))
            if( mp3GetLastError()== MP3_ERROR_OUT_OF_BUFFER)
              break;            

         inpPtr += param.inputSize;
		     inpBytes -= param.inputSize;
         outPtr+=param.outputSize;
         totalout+=param.outputSize;
      };
		  memmove(inpbuf, inpPtr, inpBytes);
  	  retval = read_file(__argv, &inpbuf[inpBytes],offset, inpsize-inpBytes, &r_bytes);
      offset+=r_bytes;	
 
	    if (r_bytes== 0)  break;	
	    
		  inpPtr = inpbuf;
		  inpBytes += r_bytes;
	    if(totalout < 32768) continue;
	  
	    outPtr = outbuf;    	
	    while (totalout > 32768)
     { wave_out(outPtr); 
	      totalout-=0x8000; 
	      outPtr+=0x8000;	
      };
      memmove(outbuf,outPtr, totalout);
      outPtr = outbuf+totalout; 
    };
    if(status != ST_EXIT)
    status =  ST_STOP;
};

void play_wave()
{
   int retval;
   int remain;
   int i;

   offset = 44;

   read_file (__argv,outbuf,offset,32*1024,0);
   offset+=32*1024;
   SetBuffer(hSound,hBuff,outbuf,0,0x8000);

   read_file (__argv,outbuf,offset,32*1024,0);
   offset+=32*1024;
   SetBuffer(hSound,hBuff,outbuf,0x8000,0x8000);

   PlayBuffer(hSound, hBuff);

   retval = 0;
   while(1)
   {
     if(status!=ST_PLAY)
       break;

     GetNotify(&event[0]);
     if(retval == FILE_EOF)
       break;
     remain = fileinfo.size-offset;
     if(remain >=32768)
     { retval = read_file (__argv,outbuf,offset,32*1024,0);
       offset+=32*1024;
       SetBuffer(hSound,hBuff,outbuf,event[1],0x8000);
       continue;
     };
     if(remain == 0)
     { retval = FILE_EOF;
       continue;
     };
     read_file (__argv,outbuf,offset,remain,0);
     for(i=remain;i<32768;i++)
       outbuf[i] = 0;
     SetBuffer(hSound,hBuff,outbuf,event[1],0x8000);
     retval= FILE_EOF;
   };

   if(status != ST_EXIT)
    status =  ST_STOP;
};

void snd_stop()
{
  StopBuffer(hSound, hBuff);
};

void thread_proc()
{ int evnt;
   int pos;
   int key;

  _asm { fninit };
 
    
  draw_window();

  while(1)
  {  if(status==ST_PLAY)
      {  draw_progress_bar();
          evnt = wait_for_event(80);
       //   debug_out_str("BIG ERROR...\x0D\x0A\x00");          
      }
     else
        evnt = wait_for_event_infinite();

    switch(evnt)
    {
      case EV_REDRAW:
        draw_window();
        break;

      case EV_KEY:
        key = get_key();
        if(key==27)
        {   status = ST_EXIT;
            exit();
        };
        if((key==45)||key==54)
        { if(m_vol > 0)
          { m_vol--;
            SetMasterVol(hDrv,m_vol);
          };
          break;
        };
        if((key==61)||key==56)
        { if(m_vol < 90)
          { m_vol++;
            SetMasterVol(hDrv,m_vol);
          };
        };
        break;

      case EV_BUTTON:
        switch(get_button_id())
        {  case 1:
             status = ST_EXIT;
             exit();
             break;
             
           case 0x10:
             status = ST_PLAY;
             continue;

           case 0x11:
             status = ST_STOP;
             break;
//           case 0x12:
//           case 0x13:
           case 0x14:
             if(m_vol > 0)
             { m_vol--;
               SetMasterVol(hDrv,m_vol);
             };
             break;

           case 0x15:
             if(m_vol < 90)
             { m_vol++;
               SetMasterVol(hDrv,m_vol);
             };
             break;

           case 0x30:
            if(status==ST_DONE)
              break;
            if(snd_play == play_mp3)
              continue;   
            pos = (GetMousePos(REL_WINDOW)>>16)-7;
            offset = ((fileinfo.size-44)/286*pos+44)&0xFFFFFFFC;
            draw_progress_bar();
            break;
        };
    };
  };
};

DWORD test_wav(WAVEHEADER *hdr)
{
  if(hdr->riff_id != 0x46464952)
    return 0;

  if(hdr->riff_format != 0x45564157)
    return 0;

  if (hdr->wFormatTag != 0x01)
    return 0;

  switch(hdr->nSamplesPerSec)
  { case 48000:
      switch (hdr->nChannels)
      {  case 1:
           if(hdr->wBitsPerSample == 16)
             return PCM_1_16_48;
           else
             return PCM_1_8_48;

         case 2:
           if(hdr->wBitsPerSample == 16)
             return PCM_2_16_48;
           else
             return PCM_2_8_48;
      };

    case 44100:
      switch (hdr->nChannels)
      {  case 1:
           if(hdr->wBitsPerSample == 16)
             return PCM_1_16_44;
           else
             return PCM_1_8_44;

         case 2:
           if(hdr->wBitsPerSample == 16)
             return PCM_2_16_44;
           else
             return PCM_2_8_44;
      };

    case 32000:
      switch (hdr->nChannels)
      {  case 1:
           if(hdr->wBitsPerSample == 16)
             return PCM_1_16_32;
           else
             return PCM_1_8_32;

         case 2:
           if(hdr->wBitsPerSample == 16)
             return PCM_2_16_32;
           else
             return PCM_2_8_32;
      };

    case 24000:
      switch (hdr->nChannels)
      {  case 1:
           if(hdr->wBitsPerSample == 16)
             return PCM_1_16_24;
           else
             return PCM_1_8_24;

         case 2:
           if(hdr->wBitsPerSample == 16)
             return PCM_2_16_24;
           else
             return PCM_2_8_24;
      };

    case 22050:
      switch (hdr->nChannels)
      {  case 1:
           if(hdr->wBitsPerSample == 16)
             return PCM_1_16_22;
           else
             return PCM_1_8_22;

         case 2:
           if(hdr->wBitsPerSample == 16)
             return PCM_2_16_22;
           else
             return PCM_2_8_22;
      };

    case 16000:
      switch (hdr->nChannels)
      {  case 1:
           if(hdr->wBitsPerSample == 16)
             return PCM_1_16_16;
           else
             return PCM_1_8_16;

         case 2:
           if(hdr->wBitsPerSample == 16)
             return PCM_2_16_16;
           else
             return PCM_2_8_16;
      };

    case 12000:
      switch (hdr->nChannels)
      {  case 1:
           if(hdr->wBitsPerSample == 16)
             return PCM_1_16_12;
           else
             return PCM_1_8_12;

         case 2:
           if(hdr->wBitsPerSample == 16)
             return PCM_2_16_12;
           else
             return PCM_2_8_12;
      };

    case 11025:
      switch (hdr->nChannels)
      {  case 1:
           if(hdr->wBitsPerSample == 16)
             return PCM_1_16_11;
           else
             return PCM_1_8_11;

         case 2:
           if(hdr->wBitsPerSample == 16)
             return PCM_2_16_11;
           else
             return PCM_2_8_11;
      };

    case 8000:
      switch (hdr->nChannels)
      {  case 1:
           if(hdr->wBitsPerSample == 16)
             return PCM_1_16_8;
           else
             return PCM_1_8_8;

         case 2:
           if(hdr->wBitsPerSample == 16)
             return PCM_2_16_8;
           else
             return PCM_2_8_8;
      };
      default:
        return 0;
  };
};

void delay (int val)
{
  _asm
 {   mov   eax,5
      mov   ebx, [val]
      int   0x40
  };  
}

int wait_for_event(int time)
{ int retval;
  _asm
 {  mov  eax,23
     mov  ebx,[time]
     int  0x40
     mov [retval], eax
 };
 return retval; 
}; 
 
int wait_for_event_infinite()
{ int retval;
  _asm
  {  mov  eax,10
      int  0x40
      mov [retval], eax 
  };
  return retval; 
};

void BeginDraw()
{_asm
 { mov   eax,12
    mov   ebx, 1
    int   0x40
  };  
};

void EndDraw()
{ _asm
 { mov   eax,12
    mov   ebx, 2
    int   0x40
  };  
};

void * __cdecl memmove ( void * dst, const void * src, size_t count)
{ void * ret = dst;

  if (dst <= src || (char *)dst >= ((char *)src + count))
  {
      while (count--)
      { *(char *)dst = *(char *)src;
          dst = (char *)dst + 1;
          src = (char *)src + 1;
      }
   }
   else
    {
        dst = (char *)dst + count - 1;
        src = (char *)src + count - 1;
         while (count--)
          {  *(char *)dst = *(char *)src;
              dst = (char *)dst - 1;
              src = (char *)src - 1;
          }
    }
    return(ret);
};




//   debug_out_str(formats[fmt]);
//   debug_out_str("\x0D\x0A\x00");

//   debug_out_str("pci cmd: ");
//   debug_out_hex(info.pci_cmd);
//   debug_out_str("\x0D\x0A\x00");

//   debug_out_str("irq line: ");
//   debug_out_hex(info.irq);
//   debug_out_str("\x0D\x0A\x00");

//   debug_out_str("global control: ");
//   debug_out_hex(info.glob_cntrl);
//   debug_out_str("\x0D\x0A\x00");

//   debug_out_str("global status:  ");
//   debug_out_hex(info.glob_sta);
//   debug_out_str("\x0D\x0A\x00");


  // call _print_volume

//   debug_out_hex(whdr.nChannels);
//   debug_out_str("\x0D\x0A\x00");
//   debug_out_hex(whdr.nSamplesPerSec);
//   debug_out_str("\x0D\x0A\x00");

//   debug_out_hex(fmt);
//   debug_out_str("\x0D\x0A\x00");



