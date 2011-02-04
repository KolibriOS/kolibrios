//
//   This file is part of the AC97 mp3 player.
//   (C) copyright Serge 2006-2007 infinity_sound@mail.ru
//   (C) copyright Quantum 2007    ufmod@users.sf.net
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
#include "../ufmod-codec.h"                   /* uFMOD integration */
void exit();                                  /* uFMOD integration */

#define DOCKABLE_WINDOW
#define MP3_ERROR_OUT_OF_BUFFER  5
int m_last_error;

void _stdcall thread_proc(void *param);
int _stdcall create_thread(void *proc, void *param, int stack_size);

#ifdef DOCKABLE_WINDOW
void GetThreadInfo (char *info, int slot); //Asper+
#endif

void touch(char *buf, int size);
int mp3FindSync(byte* buf, int size, int* sync);
int stream_read_raw(struct reader *rd,unsigned char *buf, int size);

int __cdecl _stricmp (const char * dst, const char * src);
char *__cdecl strrchr (const char * string,int ch);
int _strncmp(char *src, char *dst, DWORD n); //Asper+
int _strncpy (char *dst, char *src, int n); //Asper+
void uint2str(unsigned int value, char *string); //Asper+

struct reader rd;
struct frame fr;

DWORD hDrv;
DWORD hSound;
SNDBUF hBuff;

CTRL_INFO info;

FILEINFO   fileinfo;
const char filename[256];
const char *fileext;
char full_filename[4096];

int m_vol;
int l_vol=-700;     //-7db
int r_vol=-700;
int pan =0;

DWORD status;
DWORD first_sync;
DWORD PLStatus=0; //Asper+

#ifdef DOCKABLE_WINDOW
byte thread_info[1024]; //Asper+
#endif

int tid, pl_tid;
const DWORD main_wh=92, pl_ww=300, pl_wh=382; //Asper+
DWORD pl_wx=100, pl_wy=101+92;  //Asper+
//DWORD main_wc=0x404040, main_bc=0x808080;  //Asper+ ac97snd Classic style
//DWORD main_wc=0x002040, main_bc=0x008080, main_ic=0x002040, selected_ic=0x1010F0;  //Asper+
DWORD main_wc=0x101030, main_bc=0x008080, main_ic=0x000000, selected_ic=0x1010F0;  //Asper+

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
char header_PL[] = "PLAYLIST";
char buttons_xm[]  = " Play    Stop                    Vol-    Vol+"; /* uFMOD integration */
char buttons_wav[] = " Play    Stop     <<      >>     Vol-    Vol+"; /* uFMOD integration */
char button_PL[] = "PL"; //Asper+
char *buttons_text = buttons_wav;                                     /* uFMOD integration */

void play_xm();                                                       /* uFMOD integration */
void (*snd_play)();


//Asper_____________________Play List code start_____________________________
#define PLI_BUTTON_HEIGHT    13
#define PL_MAX_SHOWN_ITEMS   (pl_wh-PLI_BUTTON_HEIGHT-40)/PLI_BUTTON_HEIGHT
#define MAX_TEXT_WIDTH       46

int currSelected, currActive, currFirstShowed;
unsigned char *pl_buff;
char pl_path[4096];
int pl_items_number;

int ShowPLContent(char *filebuffer);
int GetFileNameFromPL(const char *fbuff, int index, char *name);
void Win2Dos (char *st, int len);

void HidePLWindow()
{
   BeginDraw();
   ResizeReplaceWindow(pl_wx,pl_wy,0,0);
   EndDraw();
}

void ShowPLWindow()
{
   unsigned int i;

   BeginDraw();
   DrawWindow(pl_wx,pl_wy,pl_ww,pl_wh,main_ic,4,0,0,0);

   for (i=0; i<PL_MAX_SHOWN_ITEMS; i++)
	   make_button(7,24+i*(PLI_BUTTON_HEIGHT+1),285,PLI_BUTTON_HEIGHT, (0x10+i)|BT_NORMAL|BT_NOFRAME, main_ic);

   write_text(8,8,FONT0, header_PL, sizeof(header_PL)-1);
   ShowPLContent(pl_buff);
   EndDraw();
}

int LoadTrack(int i)
{
	if (GetFileNameFromPL(pl_buff, i, filename))
	{
		strcpy (full_filename, pl_path);
		strcat (full_filename, filename);
		return 1;
	}
	return 0;
}

void _stdcall pl_thread_proc(void *param)
{  int evnt;
   int key, button;
   DWORD tmp_x, tmp_y, i; //Asper+
   char ipc_buff[16]="";

   set_event_mask(0x47); //Asper + IPC event
   ipc_init(ipc_buff, 16);
   
  _asm
  {
    mov eax, 66
    mov ebx, 1
    mov ecx, 1
    int 0x40
  };
    
  ShowPLWindow();

  while(1)
  {
	if (PLStatus&0xF0) 
    {
		switch(PLStatus)
		{
			case 0x11:
				HidePLWindow();
			break;
			case 0x12:
				ResizeReplaceWindow(pl_wx,pl_wy,pl_ww,pl_wh);			
			break;
		}
		PLStatus&=0x0F;
	}
	switch (status)
	{
		case ST_TRACK:
			break;
		case ST_EXIT:
			PLStatus=0x00;
			exit();
	}

#ifdef DOCKABLE_WINDOW
	tmp_x = (DWORD)thread_info[35]*0x100+(DWORD)thread_info[34];
	tmp_y = (DWORD)thread_info[38]+main_wh+1;
	if (pl_wx!= tmp_x || pl_wy!=tmp_y)
	{
		pl_wx=tmp_x;
		pl_wy=tmp_y;
		ResizeReplaceWindow(pl_wx,pl_wy,-1,-1);
	}
#endif

    evnt = wait_for_event(20);

    switch(evnt)
    {
      case EV_REDRAW:
			  ShowPLWindow();
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
					 PLStatus=0x00;
					 exit();
					 break;

				 case 0x1C:  //Enter                   
					 currActive=currFirstShowed+currSelected-1;
					 status = ST_TRACK; 
					 break; 
                 case 0x47:  //Home
					 if(l_vol < 0)
					 { l_vol+=100;
					   r_vol+=100;  
					   SetVolume(hBuff,l_vol,r_vol);
					 };
				   break;
                 case 0x48:  //Up
					 if (currSelected==0) 
					 {
						 if (currFirstShowed>0)
							 currFirstShowed--;
						 ShowPLContent(pl_buff);
						 break;
					 }
					 currSelected--;
					 ShowPLContent(pl_buff);
					 break;
                 case 0x50:  //Down
					 if (currSelected+currFirstShowed > pl_items_number-2) break;
					 if (currSelected==PL_MAX_SHOWN_ITEMS-1)
					 {
						 //if (currFirstShowed<pl_items_number)
							 currFirstShowed++;
						 ShowPLContent(pl_buff);
						 break;
					 }
					 //if (currSelected<PL_MAX_SHOWN_ITEMS) 
					 currSelected++;
					 ShowPLContent(pl_buff);
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
		  button=get_button_id();
		  if (button==1)
		  {
			  PLStatus=0x00;
			  exit();
		  }
		  for (i=0;i<PL_MAX_SHOWN_ITEMS;i++)
		  {
			  if (button==0x10+i)
			  {
				  currActive=currFirstShowed+i-1;
				  status = ST_TRACK; 
				  break;
			  }
		  }
		  break;
	  case EV_IPC:
  		  *ipc_buff='\0';
		  ShowPLContent(pl_buff);
		  break;

	}
  }
}

void Win2Dos (char *st, int len) 
{
	int i;
	unsigned char ch;

	for (i=0; i<len; i++)
	{
		ch = st[i];
		if (ch>=192)
		{
			if (ch>=240) ch-=16;
			else ch-=64;
		}
		else
		{
			if (ch==168) ch = 240;
			if (ch==184) ch = 241;
			if (ch==185) ch = 252;
			if ((ch==147) || (ch==148) || (ch==171) || (ch==187)) ch = 34;
			if ((ch==150) || (ch==151)) ch = 45;
		}
		st[i]=ch;
	}
}

int GetFileNameFromPL(const char *plbuff, int index, char *name)
{
	int count=0,i=0,j=0;
	char ch;

	do{		
		ch=plbuff[i];
		if (ch!='#' && i && plbuff[i-1]=='\n')
			count++;
		if (count-1==index) 
		{
			if (j>MAX_TEXT_WIDTH || ch=='\n')
			{
				name[j-1]='\0';
				break;
			}
			if (ch=='\\') ch='/';
			name[j++]=ch;
		}
		else if (count-1>index) break;
		i++;
	}while (ch);
	if (!ch) return 0;
	return j;
}

int CountFileNamesInPL(const char *plbuff)
{
	int count=0,i=0;
	char ch;

	do{		
		ch=plbuff[i];
		if (ch!='#' && i && plbuff[i-1]=='\n')
			count++;
		i++;
	}while (ch);
	if (count) count--;
	return count;
}

int ShowPLContent(char *filebuffer)
{
	char st[MAX_TEXT_WIDTH+10]="", tmp[MAX_TEXT_WIDTH+1]="";
	unsigned int len=8,i;
	DWORD text_color;

	draw_bar(7,24,285,PLI_BUTTON_HEIGHT*(PL_MAX_SHOWN_ITEMS+2), main_ic);
	draw_bar(7,24+currSelected*(PLI_BUTTON_HEIGHT+1),285,PLI_BUTTON_HEIGHT, selected_ic);
	for (i=0; i<PL_MAX_SHOWN_ITEMS; i++)
	{
		text_color = (currFirstShowed+i==currActive?0xFFFFFF|FONT0:0x00FF20|FONT0);
		if (!GetFileNameFromPL(filebuffer, i+currFirstShowed, tmp)) break;
		uint2str(currFirstShowed+i+1, st);
		strcat(st, ". ");
		strcat(st, tmp);
		len = strlen(st);
		if (len > MAX_TEXT_WIDTH) 
			len = MAX_TEXT_WIDTH;
		write_text(11,i*(PLI_BUTTON_HEIGHT+1)+27,text_color, st, len);
	}
	return 1;
}
//Asper_____________________Play List code end_____________________________ 

void update_dinamic_content() //Asper +
{
	int len;                                                          /* uFMOD integration */
	draw_bar(7,41,286,11,main_wc);
	
	draw_bar(7,55,286,11,main_wc);
	len = strlen(filename);                                                /* uFMOD integration */
	if(len > 47) len = 47;                                              /* uFMOD integration */
	write_text(11,57,0x00FF20|FONT0, filename, len);                        /* uFMOD integration */
}

void draw_window()
{
   BeginDraw();

   DrawWindow(100,100,299,main_wh,main_wc,4,0,0,0); //Asper+   

   make_button(7,24,45,13, 0x10|BT_NORMAL,main_bc);
   make_button(56,24,45,13, 0x11|BT_NORMAL,main_bc);
   make_button(104,24,45,13, 0x12|BT_NORMAL,main_bc);
   make_button(152,24,45,13, 0x13|BT_NORMAL,main_bc);
   make_button(200,24,45,13, 0x14|BT_NORMAL,main_bc);
   make_button(248,24,45,13, 0x15|BT_NORMAL,main_bc);

   make_button(268,70,25,13, 0x16|BT_NORMAL,main_bc); //Asper+ PL button

   make_button(7,41,286,11, 0x30|BT_HIDE|BT_NOFRAME,main_wc);

   update_dinamic_content();
   write_text(8,8,FONT0, header, sizeof(header)-1);                     /* uFMOD integration */
   write_text(12,28,main_wc|FONT0,buttons_text,sizeof(buttons_wav)-1); /* uFMOD integration */
   write_text(11,27,0xA0FFA0|FONT0,buttons_text,sizeof(buttons_wav)-1); /* uFMOD integration */

   write_text(276,74,main_wc|FONT0,button_PL,sizeof(button_PL)-1); //Asper+ PL button text
   write_text(275,73,0xA0FFA0|FONT0,button_PL,sizeof(button_PL)-1); //Asper+
   EndDraw();
};

void draw_progress_bar()
{  DWORD x;
   x = (DWORD)(287.0f * (float)(rd.filepos-rd.strremain)/(float)fileinfo.size); /* uFMOD integration */
   if(x==0) return;
   draw_bar(7,41,x,11,0xA0A0A0);
   draw_bar(x+7,41,287-x,11,main_wc);
};

void debug_out_str(const char* str)
{
  while (*str != 0)
  {
    debug_out(*str);
    str++;
  }
}

int LoadPL(char *fname)
{
	DWORD fmt;
	DWORD r_bytes;
	int retval;
	int i;
//		char st[100]="";

	char *pch;

	r_bytes=0;
	pch=strrchr(fname, '/');
	if (pch)
		i=pch-fname+1;
	else
		i=strlen(fname);

	_strncpy (pl_path, fname, i);
	pl_path[i]='\0';

	if (!pl_buff)
		pl_buff = UserAlloc(0x40000);
	retval=read_file (fname,pl_buff,0,0x40000,&r_bytes);

	if ( retval && (r_bytes==0))
		return 0;

	Win2Dos(pl_buff, r_bytes);
	pl_items_number=CountFileNamesInPL(pl_buff);
/*
	debug_out_str("\n\rPlay List files number = ");
	itoa(pl_items_number, st, 10);
	debug_out_str(st);
*/
	fmt = test_m3u(pl_buff);

	if(!fmt)
	{
		debug_out_str("\n\rInvalid M3U file");
		return 0;
	}
	debug_out_str("\n\rValid M3U file");
	currSelected=currFirstShowed=0;
	currActive=-1;
	return 1;
}

int LoadFile(char *fname)
{
   DWORD fmt;
   DWORD r_bytes;
   int retval;
   int err;
   unsigned char *ttl, *cur;               /* uFMOD integration */

   debug_out_str("\n\rPlay file ");
   debug_out_str(fname); 
   debug_out_str("\n\r");

   if(get_fileinfo(fname, &fileinfo)==FILE_NOT_FOUND)
   {  debug_out_str("\n\rfile not found\n\r");
      return 0;
   }

   r_bytes=0;
   strcpy(filename, strrchr(fname,'/')+1);
   if( !(fileext = strrchr(filename,'.')))
	   return 0;

   if(!_stricmp(fileext,".m3u"))
   {
	   LoadPL(fname);
	   status=ST_TRACK;
	   return 1;
   }

   if (!testbuff)
	   testbuff = UserAlloc(4096);

   retval=read_file (fname,testbuff,0,2048,&r_bytes);
   if ( retval && (r_bytes==0))
	   return 0;

   if (!inpbuf)
   {
	   inpbuf = UserAlloc(0x10000);
	   touch(inpbuf, 0x10000);
   }
   create_reader(&rd, inpbuf, 0x10000);
   init_reader(&rd,fname);

   if(!_stricmp(fileext,".mp3"))
   {	
			fmt = test_mp3(testbuff);              
			if(!fmt)
			{
			  debug_out_str("\n\rInvalid MP3 file");
			  return 0;                     
			};
			snd_play = &play_mp3;
			outremain = 0x40000;
			if (!outbuf)
			{
				outbuf = UserAlloc(outremain);
				touch(outbuf, outremain);
			}
			make_decode_tables(32767);
			init_layer2();
			init_layer3(32);
			fr.single = -1;
			goto play;
   };
     
   if(!_stricmp(fileext,".xm")) 
   {
	   if(uFMOD_LoadSong(fname))
	   {       
	      buttons_text = buttons_xm;               /* uFMOD integration */
	      fmt = PCM_2_16_48;                       /* uFMOD integration */
	      snd_play = &play_xm;                     /* uFMOD integration */
	      ttl = uFMOD_GetTitle();                  /* uFMOD integration */
	      cur = ttl;                               /* uFMOD integration */
	      err = 0;                                 /* uFMOD integration */
	      while(*cur && *cur++ != ' ') err++;      /* uFMOD integration */
	      if(err){                                 /* uFMOD integration */
	 	      cur = fname;                             /* uFMOD integration */
			     while(*cur) cur++;                       /* uFMOD integration */
			     *cur++ = ' ';                            /* uFMOD integration */
			     *cur++ = '|';                            /* uFMOD integration */
			     *cur++ = ' ';                            /* uFMOD integration */
			     while(*ttl) *cur++ = *ttl++;             /* uFMOD integration */
		    }
		    goto play; 
		 }   
		 debug_out_str("\n\rInvalid XM file");
     return 0;
	 };
	 
	 if(!_stricmp(fileext, ".wav"))
	 {
       fmt = test_wav((WAVEHEADER*)testbuff);
	   if(fmt)
		 {                                   
		   snd_play = &play_wave;            
		   set_reader(&rd, 44);              
		   outbuf = UserAlloc(32*1024);      
		   touch(outbuf, 32768);
		   goto play;
		 }
     debug_out_str("\n\rInvalid WAV file");
     return 0;
	 };	

   debug_out_str("\n\rUnsupported file");
   return 0;

play:

   status = ST_PLAY;
   SetFormat(hBuff, fmt);
   SetVolume(hBuff,l_vol,r_vol);
   GetVolume(hBuff,&l_vol,&r_vol);

   return 1;
}

int main(int argc, char *argv[])
{
   int err, ver;
   int i;
   char ipc_msg[2]="\0\0";
    
   strcpy (full_filename, argv[1]);
   pl_items_number=0;
    
   InitHeap(1024*1024);
   
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

   if (err = CreateBuffer(PCM_2_16_48, 0, &hBuff))
   {
     debug_out_str("create buffer return error\n\r"); 
     return 0;
   }

   if (!LoadFile(full_filename))
	   return 0;
   tid=create_thread(thread_proc, 0, 4096);
   
   while(1)
   {  delay(10);
      switch(status)
      {  case ST_TRACK:
			StopBuffer(hBuff);
			if (LoadTrack(++currActive))
			{
				if (LoadFile(full_filename))
					status = ST_PLAY;
			}
			else status = ST_STOP;

			//Update ac97snd and PL windows
			i=currActive-currFirstShowed;
			if (i>PL_MAX_SHOWN_ITEMS-1)
				currFirstShowed = currActive - PL_MAX_SHOWN_ITEMS/2;
			ipc_send_msg(tid, ipc_msg);
			ipc_send_msg(pl_tid, ipc_msg);
			continue;
	  
	  case ST_PLAY:
		  snd_play();
		  continue;

	  case ST_STOP:
		  StopBuffer(hBuff);
		  status = ST_DONE;
		  continue;

	  case ST_EXIT:
		  uFMOD_StopSong();          /* uFMOD integration */
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
    for ( i = 0;i < size; i+=4096)     //alloc all pages
      a = buf[i]; 
};

DWORD test_m3u(char *buf) //Asper+
{
	char  *sign="#EXTM3U";
	return _strncmp(buf, sign, 7);
}

DWORD test_mp3(char *buf)
{  unsigned long hdr; 
    WAVEHEADER whdr; 
     
    while (1)
    {  if(rd.filepos > 102400)
          return 0; 
        if(!rd.head_read(&rd,&hdr))
                        return 0;
        if(!decode_header(&fr,hdr))
        { 
         if((hdr & 0xffffff00) == 0x49443300)
 	      {
 		    int id3length = 0;
		    id3length = parse_new_id3(&rd, hdr);
		    continue;
	      };
          rd.strpos-=3;
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
    whdr.nChannels = 2; 
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
      { if ((status != ST_EXIT) && (status != ST_STOP))
         status = ST_TRACK;
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
  
    if ((status != ST_EXIT) && (status != ST_STOP))
      status =  ST_TRACK;
};

void play_wave()
{  int count;

   set_reader(&rd,44); 
   while(1)
   {
      if(status!=ST_PLAY)
        break;

      if( count=stream_read_raw(&rd,outbuf,32768))
      {
        WaveOut(hBuff,outbuf,count);
        continue;
      }
      done = 1;
      break; 
   };

   if ((status != ST_EXIT) && (status != ST_STOP))
     status =  ST_TRACK;
};

void play_xm(){                             /* uFMOD integration */
	while(status == ST_PLAY){                 /* uFMOD integration */
		uFMOD_WaveOut(hBuff);                   /* uFMOD integration */
		delay(8);                               /* uFMOD integration */
	}                                         /* uFMOD integration */
	if ((status != ST_EXIT) && (status != ST_STOP)) status = ST_TRACK;   /* uFMOD integration */
}                                           /* uFMOD integration */

void snd_stop()
{
  StopBuffer(hBuff);
};

void _stdcall thread_proc(void *param)
{  int evnt;
   int pos;
   int key;
   DWORD offset;
   char ipc_buff[16];

   set_event_mask(0x47); //Asper + IPC event
   ipc_init(ipc_buff, 16);

  _asm
  {
    mov eax, 66
    mov ebx, 1
    mov ecx, 1
    int 0x40
  };
    
  draw_window();

  while(1)
  {
	 if(status==ST_PLAY)
     {  draw_progress_bar();
        evnt = wait_for_event(80);		
     }
     else
		 evnt = wait_for_event_infinite();

#ifdef DOCKABLE_WINDOW
     GetThreadInfo(thread_info, -1);
#endif

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
           case 0x12:
			   currActive-=2;
			   status = ST_TRACK;
			   break;
           case 0x13:
			   status = ST_TRACK;
			   break;
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

           case 0x16: //Asper+ PL button action
			   switch (PLStatus)
			   {    case 0x00: //PL not started.
						pl_tid=create_thread(pl_thread_proc, 0, 4096);              
						PLStatus=0x12;
				    break;
					case 0x01: //PL started, but hidden.
						PLStatus=0x12;
				    break;
					case 0x02: //PL started and showed.
						PLStatus=0x11;
					break;
			   }
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
		break;

	  case EV_IPC:
		  *ipc_buff='\0';
		  update_dinamic_content();
		  break;

    };
  };
};

void delay (int val)
{
  _asm
 {    
      mov   eax,5
      mov   ebx, [val]
      int   0x40
  };  
}

int wait_for_event(int time)
{ int retval;
  _asm
 {   
     mov  eax,23
     mov  ebx,[time]
     int  0x40
     mov [retval], eax
 };
 return retval; 
}; 
 
int wait_for_event_infinite()
{ int retval;
  _asm
  {   
      mov  eax,10
      int  0x40
      mov [retval], eax 
  };
  return retval; 
};

void BeginDraw()
{_asm
 {  
    mov   eax,12
    mov   ebx, 1
    int   0x40
  };  
};

void EndDraw()
{ _asm
 {  
    mov   eax,12
    mov   ebx, 2
    int   0x40
  };  
};

//Asper+_______start KolibriOS sys functions___________________
void ResizeReplaceWindow (DWORD x, DWORD y, DWORD w, DWORD h) //Asper+
{
  _asm
 {    
      mov   eax, 67
      mov   ebx, [x]
      mov   ecx, [y]
      mov   edx, [w]
      mov   esi, [h]
      int   0x40
  };  
}

#ifdef DOCKABLE_WINDOW
void GetThreadInfo (char *info, int slot) //Asper+
{
	_asm
	{    
		mov   eax, 9
		mov   ebx, [info]
		mov   ecx, [slot]
		int   0x40
	}  
}
#endif

void set_event_mask(int mask)
{
	_asm
	{   
		mov  eax, 40
		mov  ebx, [mask]
		int  0x40     
	}
}

void ipc_init(char *buf, int bufsize)
{
	_asm
	{   
		mov  eax, 60
		mov  ebx, 1
		mov  ecx, [buf]
		mov  edx, [bufsize]
		int  0x40     
	}
}

int ipc_send_msg(int PID, char *msg)
{
	int len = strlen(msg);
	int retval;
	_asm
	{   
		mov  eax, 60
		mov  ebx, 2
		mov  ecx, [PID]
		mov  edx, [msg]
		mov  esi, [len]
		int  0x40     
		mov [retval], eax 
	}
}
//Asper+_______end KolibriOS sys functions___________________

//Asper+_______start strings routines___________________
int _strncmp(char *src, char *dst, DWORD n)
{
	_asm{
		mov		esi, src
		mov		edi, dst
		mov		ecx, n
	}
 l1:
	_asm{
		cmpsb
		jne 	err
		loop 	l1	
	}
	return 1;
 err:
	return 0;
}

int _strncpy (char *dst, char *src, int n)
{
	int  i;
	for (i=0; i<n; i++)
	{
		dst[i]=src[i];
		if (src[i]=='\0') break;
	}
	return 0;
}

void uint2str(unsigned int value, char *string)
{
  char tmp[33];
  int i, j;
  unsigned v;

  v = (unsigned)value;
  j = 0;
  do{
    i = v % 10;
    v = v / 10;
    if (i < 10)
      tmp[j] = i+'0';
    else
      tmp[j] = i + 'a' - 10;
	j++;
  }while (v);

  for (i=0; i<j; i++)
    string[i] = tmp[j-i-1];
  string[i] = '\0';
}

//Asper+_______end strings routines___________________


///*********
void *memmove ( void * dst, void * src, unsigned int count)  /* uFMOD integration */
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

char * __cdecl strrchr (const char * string,int ch)
{
        char *start = (char *)string;

        while (*string++)                       /* find end of string */
                ;
                                                /* search towards front */
        while (--string != start && *string != (char)ch)
                ;

        if (*string == (char)ch)                /* char found ? */
                return( (char *)string );

        return(NULL);
}

int __cdecl _stricmp (const char * dst, const char * src)
{
    int f, l;

    do
    {
        if ( ((f = (unsigned char)(*(dst++))) >= 'A') && (f <= 'Z') )
            f -= 'A' - 'a';
        if ( ((l = (unsigned char)(*(src++))) >= 'A') && (l <= 'Z') )
            l -= 'A' - 'a';
    }
    while ( f && (f == l) );

    return(f - l);
}


