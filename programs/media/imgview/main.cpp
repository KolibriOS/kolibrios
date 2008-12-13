#include "kosSyst.h"
#include "gfx.cpp"
#include "dlgopen.h"
#include "formats\pcx.h"
#include "formats\bmp.h"
#include "formats\tga.h"

#define  btn_open    10
#define  btn_prev    11
#define  btn_next    12
#define  btn_fit     13

#define  scrl_up     30
#define  scrl_down   31
#define  scrl_left   32
#define  scrl_right  33

char  params[2048] = "PARAM";
const char header[] = "ImageView v 0.08";
int   image_opened=0;

Byte* image_buffer=NULL;  // Полное RAW-Изображение
Byte* img_cuted=NULL;     // Усеченное RAW-изображение (используется при прокрутке или масштабировании)
Byte* cur_files=NULL;     // Буфер для общего списка файлов текущей папки
Word* img_files=NULL;     // Указатель массив со список граф.файлов в текущей папке
Dword cnt_imgfiles;       // Количество элементов массива img_files[]
Dword cur_image;          // Номер текущего изображения в массиве
char  cur_folder[512];    // Путь к текущей папке с / на конце

Dword image_width;
Dword image_height;
Dword old_status_size=0;

char szStatus[256];
bool scale=0;


sProcessInfo proc;
Dword img_area_left,img_area_top,img_area_width,img_area_height;
int vscrl_max=100,vscrl_value=0,vscrl_coord_x,vscrl_coord_y,vscrl_coord_min,vscrl_coord_max,vscrl_using=0;
int hscrl_max=100,hscrl_value=0,hscrl_coord_x,hscrl_coord_y,hscrl_coord_min,hscrl_coord_max,hscrl_using=0;
Dword width_old,height_old;

/* Вырезает квадрат из изображения по координатам*/
void GetRectImage(Byte* src, Byte* dst, Dword left,Dword top,Dword width,Dword height)
{
  int x,y;
  int pImgS,pImgD=0;
  
  for(y=top;y<top+height;y++)
  {
    for(x=left;x<left+width;x++)
    {
      pImgS=y*image_width*3+x*3;
      *(dst+pImgD+0)=*(src+pImgS+0);
      *(dst+pImgD+1)=*(src+pImgS+1);
      *(dst+pImgD+2)=*(src+pImgS+2);
      pImgD=pImgD+3;
    }
  }
}

/* Растягивает изображение в меньшую сторону (иначе нужно выделять больше памяти под img_cuted) */
void ResizeImage(Byte* src, Byte* dst, Dword new_width,Dword new_height)
{  
  int x,y,src_y,src_x;
  int pImgS,pImgD=0;
  for(y=0;y<new_height;y++)
  {
    for(x=0;x<new_width;x++)
    {
      __asm
      { 
        finit
        fild   image_width
        fidiv  new_width
        fild   x
        fmul   st(0),st(1)
        fistp  src_x
        fild   image_height
        fidiv  new_height
        fild   y
        fmul   st(0),st(1)
        fistp  src_y
      }
      pImgS=src_y*image_width*3+src_x*3;
      pImgD=y*new_width*3+x*3;
      *(dst+pImgD+0)=*(src+pImgS+0);
      *(dst+pImgD+1)=*(src+pImgS+1);
      *(dst+pImgD+2)=*(src+pImgS+2);
    }
  }
}

void set_vscroll_values(Dword max, Dword value)
{
  vscrl_max=max;
  vscrl_value=value;
  kos_DrawBar(vscrl_coord_x,vscrl_coord_y,15,35,0xDADADA);
  vscrl_coord_y=vscrl_coord_min+((value*(vscrl_coord_max-vscrl_coord_min-35))/vscrl_max);
  kos_PutImage(scroll_v,15,35,vscrl_coord_x,vscrl_coord_y);
}

void set_hscroll_values(Dword max, Dword value)
{
  hscrl_max=max;
  hscrl_value=value;
  kos_DrawBar(hscrl_coord_x,hscrl_coord_y,35,15,0xDADADA);
  hscrl_coord_x=hscrl_coord_min+((value*(hscrl_coord_max-hscrl_coord_min-35))/hscrl_max);
  kos_PutImage(scroll_h,35,15,hscrl_coord_x,hscrl_coord_y);
}

void draw_image()
{
  Dword new_width,new_height,scale_left,scale_top;
  if (!image_opened) return;
  
  if (scale==0)
  {
    /* Здесь вывод изображения с использованием полос прокрутки */
    new_width = image_width <= img_area_width ? image_width : img_area_width;
    new_height = image_height <= img_area_height ? image_height : img_area_height;
    GetRectImage(image_buffer, img_cuted, hscrl_value, vscrl_value, new_width, new_height);
    kos_PutImage((RGB*)img_cuted, new_width, new_height, img_area_left,img_area_top);
  } else {
    /* Здесь вывод изображения с использованием масштабирования, сохраняя пропорции */
    __asm
    {
      finit
      fild   img_area_width
      fidiv  image_width
      fild   image_width
      fmul   st(0),st(1)
      fistp  new_width
      fild   image_height
      fmul   st(0),st(1)
      fistp  new_height
    }
    if (img_area_height<new_height)
    {
      __asm
      {
        finit
        fild   img_area_height
        fidiv  image_height
        fild   image_width
        fmul   st(0),st(1)
        fistp  new_width
        fild   image_height
        fmul   st(0),st(1)
        fistp  new_height
      }
    }
    
    /* это в случае не пропорционального масштабирования */
    //new_width=img_area_width;
    //new_height=img_area_height;

    if (new_width>image_width) new_width=image_width;
    if (new_height>image_height) new_height=image_height;
  
    ResizeImage(image_buffer,img_cuted,new_width,new_height);
    /* Центрирование изображения */
    scale_left=(img_area_width/2)-(new_width/2);
    scale_top=(img_area_height/2)-(new_height/2);
    kos_PutImage((RGB*)img_cuted,new_width,new_height,img_area_left+scale_left,img_area_top+scale_top);
  }
}

void draw_window(void)
{
  kos_WindowRedrawStatus(1);
  kos_DefineAndDrawWindow(0,0,450,350,0x33,0xEFEBEF,0,0,(Dword)header); 
  
  //ToolBar
    //open
    kos_DefineButton(2,4,20,20,btn_open+0x40000000,0xEFEBEF);
    kos_PutImage(tbOpen,21,21,2,4);
    //prev
    kos_DefineButton(25,4,20,20,btn_prev+0x40000000,0xEFEBEF);
    kos_PutImage(tbPrev,21,21,25,4);
    //next
    kos_DefineButton(48,4,20,20,btn_next+0x40000000,0xEFEBEF);
    kos_PutImage(tbNext,21,21,48,4);
    //fit image
    kos_DefineButton(71,4,20,20,btn_fit+0x40000000,0xEFEBEF);   
    kos_PutImage(tbFit,21,21,71,4);
    
  //Lines
    kos_ProcessInfo(&proc,-1);
    kos_DrawBar(2,30,proc.processInfo.work_width-3,1,0x94AECE);
    
  //Scroll controls
    //vertical scroll bar
    vscrl_coord_x=proc.processInfo.work_width-16;
    vscrl_coord_max=proc.processInfo.work_height-46;
    kos_DrawBar(proc.processInfo.work_width-16,33,15,proc.processInfo.work_height-79,0xDADADA);
    kos_DefineButton(proc.processInfo.work_width-16,33,14,14,scrl_up+0x40000000,0xEFEBEF);
    kos_DefineButton(proc.processInfo.work_width-16,proc.processInfo.work_height-46,14,14,scrl_down+0x40000000,0xEFEBEF);
    kos_PutImage(arrow_up,15,15,proc.processInfo.work_width-16,33);
    kos_PutImage(arrow_down,15,15,proc.processInfo.work_width-16,proc.processInfo.work_height-46);
    kos_PutImage(scroll_v,15,35,vscrl_coord_x,vscrl_coord_y);
    
    
    //horisontal scroll bar  
    hscrl_coord_y=proc.processInfo.work_height-31;
    hscrl_coord_max=proc.processInfo.work_width-31;
    kos_DrawBar(2,proc.processInfo.work_height-31,proc.processInfo.work_width-18,15,0xDADADA);
    kos_DefineButton(2,proc.processInfo.work_height-31,14,14,scrl_left+0x40000000,0xEFEBEF);
    kos_DefineButton(proc.processInfo.work_width-31,proc.processInfo.work_height-31,14,14,scrl_right+0x40000000,0xEFEBEF);
    kos_PutImage(arrow_left,15,15,2,proc.processInfo.work_height-31);
    kos_PutImage(arrow_right,15,15,proc.processInfo.work_width-31,proc.processInfo.work_height-31);
    kos_PutImage(scroll_h,35,15,hscrl_coord_x,hscrl_coord_y);
  
  img_area_left=2;
  img_area_top=33;
  img_area_width=proc.processInfo.work_width-img_area_left-3-16;
  img_area_height=proc.processInfo.work_height-img_area_top-3-16-15;
  
  kos_WriteTextToWindow(2,proc.processInfo.work_height-12, 0x80, 0, szStatus, 0);
   
  if (img_area_width<image_width) set_hscroll_values(image_width-img_area_width,0); else set_hscroll_values(1,0);
  if (img_area_height<image_height) set_vscroll_values(image_height-img_area_height,0); else set_vscroll_values(1,0);

  draw_image();

  kos_WindowRedrawStatus(2);
}

void LoadPCX(Byte* filebuff, Dword filesize)
{
  PCXFile pcx;
  if (!pcx.LoadPCXFile(filebuff,filesize))
  {
    image_opened=1;
    image_buffer=pcx.buffer;
    image_width=pcx.width;
    image_height=pcx.height;
  } else { 
    image_opened=0; 
  }
}

void LoadBMP(Byte* filebuff, Dword filesize)
{
  BMPFile bmp;
  if (!bmp.LoadBMPFile(filebuff,filesize))
  {
    image_opened=1;
    image_buffer=bmp.buffer;
    image_width=bmp.width;
    image_height=bmp.height;
  } else { 
    image_opened=0; 
  }
}

void LoadTGA(Byte* filebuff, Dword filesize)
{
  TGAFile tga;
  if (!tga.LoadTGAFile(filebuff,filesize))
  {
    image_opened=1;
    image_buffer=tga.buffer;
    image_width=tga.width;
    image_height=tga.height;
  } else { 
    image_opened=0; 
  }
}

//Загрузить список файлов, поддерживаемых программой из папки с текущим файлом
void load_folder(char* imgfile)
{
  kosFileInfo fi;
  char* cPtr;
  int i;
  
  // Извлечем имя папки
  strcpy(fi.fileURL, imgfile);
  cPtr=strrchr(fi.fileURL,'/');
  cPtr[1]=0;
  strcpy(cur_folder,fi.fileURL);
  
  if (cur_files) { kos_FreeMemory(cur_files); cur_files=NULL;}
  if (img_files) { kos_FreeMemory((Byte*)img_files); img_files=NULL;}
  
  /* Выделим память под файл */ 
  cur_files = kos_GetMemory(65536);
  fi.rwMode = 1;
  fi.OffsetLow = 0;
  fi.OffsetHigh = 0;
  fi.dataCount = 65536;
  fi.bufferPtr = cur_files;
  int state = kos_FileSystemAccess(&fi);
  if (state!=0 && state!=6) {kos_FreeMemory(cur_files); cur_files=NULL; return;}
  
  Dword file_count=*(cur_files+8);
  img_files = (Word*)kos_GetMemory(4*file_count);
  
  char* image_name;
  cnt_imgfiles=0;
  cur_image=-1;
  
  lcase(imgfile);
  for(i=0;i<file_count;i++)
  {
    image_name=(char*)cur_files+32+i*304+40;
    lcase(image_name);
    cPtr=strrchr(image_name,'.');
    if (strcmp((char*)cPtr,".bmp") || strcmp((char*)cPtr,".pcx") || strcmp((char*)cPtr,".tga")) 
    {
      cPtr=strrchr(imgfile,'/');
      if (strcmp(cPtr+1,image_name)) cur_image=cnt_imgfiles;
      ((Word*)img_files)[cnt_imgfiles]=i;
      cnt_imgfiles++;
    }
  }
  cnt_imgfiles--;
}

void open_imgfile(char* imgfile)
{
  Word scr_left,scr_top,scr_right,scr_bot;
  Word win_width,win_height;
  Dword skin_height;
  kosFileInfo fi;
  Dword file_size;
  Byte* filebuff;
  Byte* infoBuf;
  

  /* Вычислим размер файла */
  infoBuf = kos_GetMemory(40); /* Буфер для инфо-структуры */
  fi.rwMode = 5;
  fi.OffsetLow = 0;
  fi.OffsetHigh = 0;
  fi.dataCount = 0;
  fi.bufferPtr = infoBuf;
  strcpy(fi.fileURL, imgfile);
  if (kos_FileSystemAccess(&fi)) { kos_FreeMemory(infoBuf); return;}
  file_size=*(Dword*)(infoBuf+32);
  kos_FreeMemory(infoBuf);
  
  /* Выделим память под файл */
  filebuff = kos_GetMemory(file_size);
  fi.rwMode = 0;
  fi.OffsetLow = 0;
  fi.OffsetHigh = 0;
  fi.dataCount = file_size;
  fi.bufferPtr = filebuff;
  strcpy(fi.fileURL, imgfile);
  if (kos_FileSystemAccess(&fi)) { kos_FreeMemory(filebuff); return;}

  /* Определяем тип файла */
  //lcase(imgfile); уже в нижнем регистре: это выполняется в load_folder
  char* cPtr;
  cPtr=strrchr(imgfile,'.');
  if (strcmp((char*)cPtr,".bmp")) LoadBMP(filebuff,file_size);
  if (strcmp((char*)cPtr,".pcx")) LoadPCX(filebuff,file_size);
  if (strcmp((char*)cPtr,".tga")) LoadTGA(filebuff,file_size);
  cPtr=strrchr(imgfile,'/')+1;
  if (image_opened)
  {
    sprintf(szStatus,"” ©« %S... [%U/%U]",cPtr,cur_image+1,cnt_imgfiles+1);
  
    __asm
    {
      mov  eax,48
      mov  ebx,5
      int  40h
      mov  scr_right,ax
      shr  eax,16
      mov  scr_bot,bx
      shr  ebx,16
      mov  scr_left,ax
      mov  scr_top,bx
      mov  eax,48
      mov  ebx,4
      int  40h
      mov  skin_height,eax
    }
    
    if (scr_right-scr_left+1>image_width) win_width=image_width+img_area_left+3+16+10; else win_width=scr_right-scr_left+1;
    if (scr_bot-scr_top+1>image_height) win_height=image_height+img_area_top+3+16+15+5+skin_height; else win_height=scr_bot-scr_top+1;
    
    if (win_width<150) win_width=150;
    if (win_height<160) win_height=160;
    
    img_cuted=kos_GetMemory(image_width*image_height*3);   
    kos_ChangeWindow(scr_left,scr_top,win_width,win_height);
    
    if (img_area_width<image_width) set_hscroll_values(image_width-img_area_width,0); else set_hscroll_values(1,0);
    if (img_area_height<image_height) set_vscroll_values(image_height-img_area_height,0); else set_vscroll_values(1,0);
  } else {
    sprintf(szStatus,"” ©« %S ­Ґ § Јаг¦Ґ­. ”®а¬ в ­Ґ Ї®¤¤Ґа¦Ёў Ґвбп.",cPtr);
  }
  draw_window();
  kos_FreeMemory(filebuff);
}

void show_dlg()
{
  char* imgfile;
  imgfile=DialogOpenFile(&draw_window);
  if (!imgfile) return;
  load_folder(imgfile);
  //Если файл уже был открыт...
  if (image_opened)
  {
    image_opened=0;
    kos_FreeMemory(image_buffer); image_buffer=NULL;
    kos_FreeMemory(img_cuted); img_cuted=NULL;
  }
  open_imgfile(imgfile);
}

void kos_Main()
{
  Dword btnID;
  Byte keyCode;
  Dword mButton;
  int  mX,mXOld;
  int  mY,mYOld;
  char label1[100];
  
  /* Инициализация кучи процесса */
  __asm
  { 
    mov  eax,68
    mov  ebx,11
    int  40h 
  }
  
  vscrl_coord_min=vscrl_coord_y=33+15;
  hscrl_coord_min=hscrl_coord_x=2+15;
  strcpy(szStatus,"” ©« ­Ґ § Јаг¦Ґ­");
  
  draw_window();
  if (params[0]!='P') {load_folder(params); open_imgfile(params);}
  kos_SetMaskForEvents(0x27);
  for (;;)
  {    
    switch (kos_WaitForEvent(10))
    {
       case 1:
         draw_window();
       break;
       
       case 2:
         kos_GetKey(keyCode);
       break;
       
       case 3:
         kos_GetButtonID(btnID);
         switch (btnID)
         {
           case 1:
             //Освободим память
             if (image_buffer) kos_FreeMemory(image_buffer);
             if (img_cuted) kos_FreeMemory(img_cuted);
             if (cur_files) kos_FreeMemory(cur_files);
             if (img_files) kos_FreeMemory((Byte*)img_files);
             kos_ExitApp();
           break;
           
           case btn_open:
             show_dlg();
           break;
           
           //Используем уже не нужный нам params
           case btn_prev:
             if (!cur_files) break;
             if (!cur_image) cur_image=cnt_imgfiles; else cur_image--;
             strcpy(params,cur_folder);
             strcpy(params+strlen(cur_folder),(char*)cur_files+32+((Word*)img_files)[cur_image]*304+40);
             open_imgfile(params);
           break;
           
           case btn_next:
             if (!cur_files) break;
             if (cur_image==cnt_imgfiles) cur_image=0; else cur_image++;  
             strcpy(params,cur_folder);
             strcpy(params+strlen(cur_folder),(char*)cur_files+32+((Word*)img_files)[cur_image]*304+40);
             open_imgfile(params);
           break;

/*
           case scrl_up:
             if (vscrl_max==1 || vscrl_coord_y<=vscrl_coord_min) break;
             set_vscroll_values(vscrl_max, ((vscrl_coord_y - 2 - vscrl_coord_min) * vscrl_max) / (vscrl_coord_max - vscrl_coord_min - 35));
           break;
           
           case scrl_down:
             if (vscrl_max == 1 || vscrl_coord_y >= vscrl_coord_max) break;
             set_vscroll_values(vscrl_max, ((vscrl_coord_y + 2 - vscrl_coord_min) * vscrl_max) / (vscrl_coord_max - vscrl_coord_min - 35));
           break;
*/

           case btn_fit:
             scale = scale ? 0 : 1;
             draw_window();
           break;

         }
       break;

       case 6:
       default:
           kos_GetMouseState(mButton,mX,mY);
           //Вертикальная прокрутка
           if (mButton==1)
           {
             if (vscrl_using)
             {
               kos_DrawBar(vscrl_coord_x,vscrl_coord_y,15,35,0xDADADA);
               if (vscrl_coord_y+mY-mYOld<vscrl_coord_min)
               {
                 vscrl_value=0;
                 vscrl_coord_y=vscrl_coord_min;
               } else if (vscrl_coord_y+35+mY-mYOld>vscrl_coord_max) {
                 vscrl_value=vscrl_max;
                 vscrl_coord_y=vscrl_coord_max-35;
               } else {
                 vscrl_value=(((vscrl_coord_y-vscrl_coord_min)*vscrl_max)/(vscrl_coord_max-vscrl_coord_min-35));
                 vscrl_coord_y=vscrl_coord_y+mY-mYOld;
                 mYOld=mY;
               }
               kos_PutImage(scroll_v,15,35,vscrl_coord_x,vscrl_coord_y);
               draw_image();
             } else {
               if (mY>=vscrl_coord_y && mY<=vscrl_coord_y+35 && mX>=vscrl_coord_x && mX<=vscrl_coord_x+15)
               {
                 vscrl_using=1;
                 mYOld=mY;
               }
             }
           } else if(mButton==0) {
             if (vscrl_using) {vscrl_using=0; draw_image();}
           }
           //Горизонтальная прокрутка
           if (mButton==1)
           {
             if (hscrl_using)
             {
               kos_DrawBar(hscrl_coord_x,hscrl_coord_y,35,15,0xDADADA);
               if (hscrl_coord_x+mX-mXOld<hscrl_coord_min)
               {
                 hscrl_value=0;
                 hscrl_coord_x=hscrl_coord_min;
               } else if (hscrl_coord_x+35+mX-mXOld>hscrl_coord_max) {
                 hscrl_value=hscrl_max;
                 hscrl_coord_x=hscrl_coord_max-35;
               } else {
                 hscrl_value=(((hscrl_coord_x-hscrl_coord_min)*hscrl_max)/(hscrl_coord_max-hscrl_coord_min-35));
                 hscrl_coord_x=hscrl_coord_x+mX-mXOld;
                 mXOld=mX;
               }
               kos_PutImage(scroll_h,35,15,hscrl_coord_x,hscrl_coord_y);
               draw_image();
             } else {
               if (mX>=hscrl_coord_x && mX<=hscrl_coord_x+35 && mY>=hscrl_coord_y && mY<=hscrl_coord_y+15) 
               {
                 hscrl_using=1;
                 mXOld=mX;
               }
             }
           } else if(mButton==0) {
             if (hscrl_using) {hscrl_using=0; draw_image();}
           }
         
       break;
    }
  }
}