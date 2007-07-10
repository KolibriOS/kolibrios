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

#include "../kolibri.h"
#include "string.h"
#include "ac97wav.h"
#include "../mpg/mpg123.h"
#include "../sound.h"


#define MP3_ERROR_OUT_OF_BUFFER  5
int m_last_error;

void _stdcall thread_proc(void *param);
void _stdcall create_thread(void *proc, void *param, int stack_size);
void _stdcall send_ipc(int dst, DWORD code);

void touch(char *buf, int size);
int mp3FindSync(byte* buf, int size, int* sync);
int stream_read_raw(struct reader *rd,unsigned char *buf, int size);

char *fname;

struct reader rd;
struct frame fr;

DWORD hDrv;
DWORD hSound;
SNDBUF hBuff;

CTRL_INFO info;

FILEINFO   fileinfo;

int m_vol;
int l_vol=-700;     //-7db
int r_vol=-700;
int pan =0;

DWORD status;
DWORD first_sync;

unsigned char *testbuff;
unsigned char *outbuf;
unsigned char *inpbuf;
unsigned char *outPtr;

int inpsize;
int outsize;
int outremain;
int totalout;
int done;
 
char header[] = "AC97 MP3 player";
char buttons_text[]=" Play    Stop     <<      >>     Vol-    Vol+";

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
   write_text(12,58,0x004000|FONT0, fname, strlen(fname));
   write_text(11,57,0x00FF20|FONT0, fname, strlen(fname));

   write_text(8,8,0xFFFFFF|FONT0, header, strlen(header));
   write_text(12,28,0x404040|FONT0,buttons_text,strlen(buttons_text));
   write_text(11,27,0xA0FFA0|FONT0,buttons_text,strlen(buttons_text));

   EndDraw();
};

void draw_progress_bar()
{  DWORD x;
   x = 287.0f * (float)(rd.filepos-rd.strremain)/(float)fileinfo.size;
   if(x==0) return;
   draw_bar(7,41,x,11,0xA0A0A0);
   draw_bar(x+7,41,287-x,11,0x404040);
};

void debug_out_str(char* str)
{
  while (*str != 0)
  {
    debug_out(*str);
    str++;
  }
}

int main(int argc, char *argv[])      //int argc, char *argv[])
{
   DWORD fmt;
   DWORD r_bytes;
   int retval;
   int err;
   int ver;
    
   fname = argv[1];
   debug_out_str("\n\rPlay file ");
   debug_out_str(fname); 
   debug_out_str("\n\r");
    
   InitHeap(1024*1024);
   if(get_fileinfo(fname, &fileinfo)==FILE_NOT_FOUND)
   {  debug_out_str("\n\rfile not found\n\r"); 
      return 0;
   };
   
   if(err = InitSound(&ver))
   {  
     debug_out_str("Sound service not installed\n\r"); 
     return 0;
   }
   
   if( (SOUND_VERSION>(ver&0xFFFF)) ||
       (SOUND_VERSION<(ver >> 16)))
   {  
     debug_out_str("Sound service version mismatch\n\r"); 
     return 0;
   }
   
   testbuff = UserAlloc(4096); 
   
   get_fileinfo(fname, &fileinfo);
   r_bytes=0;
   retval=read_file (fname,testbuff,0,2048,&r_bytes);
   if ( retval && (r_bytes==0)) 
     return 0;
    
   inpbuf = UserAlloc(0x10000);
   touch(inpbuf, 0x10000);
   
   create_reader(&rd, inpbuf, 0x10000);
   init_reader(&rd,fname);

   fmt = test_wav((WAVEHEADER*)testbuff);
   if (fmt != 0)
   {
     snd_play = &play_wave;
     set_reader(&rd, 44);
     outbuf = UserAlloc(32*1024);
     touch(outbuf, 32768);
   }   
   else  
   { fmt = test_mp3(testbuff);
     if(fmt ==0) return 0;
     snd_play = &play_mp3;
      
     outremain = 0x40000 ;
     outbuf = UserAlloc(outremain);
     touch(outbuf, outremain);
     make_decode_tables(32767);
     init_layer2();
     init_layer3(32);
     fr.single = -1;
   };

   status = ST_PLAY;
   
   if (err = CreateBuffer(fmt,0, &hBuff))
   {
     debug_out_str("create buffer return error\n\r"); 
    ; return 0;
   }
       
   SetVolume(hBuff,l_vol,r_vol);
   GetVolume(hBuff,&l_vol,&r_vol); 
 
   create_thread(thread_proc, 0, 4096);
   
   while(1)
   {  delay(10);
      switch(status)
      {  case ST_PLAY:
           snd_play();
           continue;

         case ST_STOP:
           StopBuffer(hBuff);
           status = ST_DONE;
           continue;

         case ST_EXIT:
           StopBuffer(hBuff);
           DestroyBuffer(hBuff);
           return 0;
      };
   };
   return 0;
};

void touch(char *buf, int size)
{ int i;
   char a;
    for ( i = 0;i < size; i+=4096)
      a = buf[i]; 
};

DWORD test_mp3(char *buf)
{  unsigned long hdr; 
    WAVEHEADER whdr; 
     
    while (1)
    {  if(rd.filepos > 102400)
          return 0; 
        if(!rd.head_read(&rd,&hdr))
                        return 0;
        if(!decode_header(&fr,hdr))
        {  rd.strpos-=3;
            rd.stream-=3;
            rd.strremain+=3;
            continue;
        };
        break;
          };
          
    first_sync = rd.filepos-rd.strremain-4;
          
    whdr.riff_id = 0x46464952;
    whdr.riff_format = 0x45564157;
    whdr.wFormatTag = 0x01;
    whdr.nSamplesPerSec = freqs[fr.sampling_frequency];
    whdr.nChannels = 2; //mpginfo.channels;
    whdr.wBitsPerSample = 16;
    
    return test_wav(&whdr);
};


void play_mp3()
{  char *outPtr;
    int totalout;
    int outcount;

 //   memset(&fr,0,sizeof(fr));
    fr.down_sample_sblimit = 32;
    fr.single = -1;
    reset_mpg();

    outPtr = outbuf;
    totalout=0;
    done = 0;
    outremain=0x40000;

    memset(outbuf,0,0x40000); 
    set_reader(&rd, 0);    //;first_sync);

    while(1)
    { if(status!=ST_PLAY)
             break;
  
     for(;;)
     {   outcount = 0;                          
          if( !read_frame(&rd, &fr))
          {  done = 1;
              break; 
          }; 
          fr.do_layer(&fr, outPtr,&outcount);
          outPtr+= outcount;
          totalout+=outcount;
          outremain-=outcount; 
          if(outremain < outcount*2)
            break;   
    };
  
    if(done)
    { if(totalout < 4096)
      {  memset(outPtr,0,4096-totalout); 
                totalout = 4096;
      };
    }
    else
      if(totalout < 8192)
        continue;

    outPtr = outbuf;      
    while (totalout >= 4096)
    { 
    
      WaveOut(hBuff,outPtr,4096);
      if(status!=ST_PLAY)
      { if(status != ST_EXIT)
         status =  ST_STOP;
        return; 
      };
      totalout-=4096; 
      outPtr+=4096;
      outremain+=4096; 
    };
    if(done)
      break;
      
    memmove(outbuf,outPtr, totalout);
    outPtr = outbuf+totalout;
   } 
  
    if(status != ST_EXIT)
      status =  ST_STOP;
};

void play_wave()
{  int retval;

   set_reader(&rd,44); 
   retval = 0;
   while(1)
   {
      if(status!=ST_PLAY)
        break;

      if( !stream_read_raw(&rd,outbuf,32768))
      {  done = 1;
          break; 
      }; 
      WaveOut(hBuff,outbuf,32768);
   };

   if(status != ST_EXIT)
    status =  ST_STOP;
};

void snd_stop()
{
  StopBuffer(hBuff);
};

void _stdcall thread_proc(void *param)
{  int evnt;
   int pos;
   int key;
   DWORD offset;
   
  _asm
  {
    mov eax, 66
    mov ebx, 1
    mov ecx, 1
    int 0x40
  };
    
  draw_window();

  while(1)
  {  if(status==ST_PLAY)
     {  draw_progress_bar();
        evnt = wait_for_event(80);
     }
     else
        evnt = wait_for_event_infinite();

    switch(evnt)
    {
      case EV_REDRAW:
        draw_window();
        break;

      case EV_KEY:
        if(!get_key(&key))
        { 
        
          switch(key)
          {  case 0xE0:
             case 0xE1:
               break;
             default:
               switch (key)
               {
                 case 0x01:  //Esc
                   status = ST_EXIT;
                   exit();
                   break; 
               
                 case 0x47:  //Home
                   if(l_vol < 0)
                   { l_vol+=100;
                     r_vol+=100;  
                     SetVolume(hBuff,l_vol,r_vol);
                   };
                   break;
                 case 0x4F:  //End                
                   if(l_vol > -10000)
                   { l_vol-=100;
                     r_vol-=100;  
                     SetVolume(hBuff,l_vol,r_vol);
                   }; 
                   break;
                 case 0x53:
                   if(pan > -10000)
                   { pan -=100;
                     SetPan(hBuff,pan);
                   };
                   break;   
                 case 0x51:
                   if(pan < 10000)
                   { pan +=100;
                     SetPan(hBuff,pan);
                   };
                   break;   
               } 
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
            if(l_vol > -10000)
            {
              l_vol-=100;
              r_vol-=100;  
              SetVolume(hBuff,l_vol,r_vol);
            };
            break;

           case 0x15:
            if(l_vol < 0)
            { l_vol+=100;
              r_vol+=100;  
              SetVolume(hBuff,l_vol,r_vol);
            };
            break;

           case 0x30:
            if(status==ST_DONE)
              break;
            pos = (GetMousePos(REL_WINDOW)>>16)-7;
            offset = ((fileinfo.size-44)/286*pos+44)&0xFFFFFFFC;
            set_reader(&rd, offset); 
            draw_progress_bar();
            break;
        };
    };
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

///*********
void *memmove ( void * dst, void * src, int count)
{ void *ret;
  ret = dst;

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
    return ret;
};
//**********/

void * __cdecl mem_cpy(void * dst,const void * src,size_t count)
{    void * ret = dst;
      while (count--)
      {  *(char *)dst = *(char *)src;
          dst = (char *)dst + 1;
          src = (char *)src + 1;
      };
      return(ret);
}

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



