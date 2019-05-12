#include "..\kosSyst.h"
#include "tga.h"

int TGAFile::LoadTGAFile(Byte* filebuff, Dword filesize)
{
  
  memcpy((Byte*)&Tga_head,(Byte*)filebuff,sizeof(sTGAHeader));
  width=Tga_head.Width;
  height=Tga_head.Height;
  
  Byte* pImg=filebuff+sizeof(sTGAHeader)+Tga_head.BytesInIdentField+Tga_head.ColorMapOrigin+(Tga_head.ColorMapLength*Tga_head.ColorMapEntrySize/8);
  Byte* pPal=filebuff+sizeof(sTGAHeader)+Tga_head.BytesInIdentField+Tga_head.ColorMapOrigin;
  Byte* cBuffer;
  int x,y;
  Byte  r;
  int sm;
      
  int state=1;
  
    if (Tga_head.ImageDescByte >= 32) sm=height-1; else sm=0;
    // Изображение с палитрой (палитра 24 или 32-битная)
    if (Tga_head.ImageTypeCode==1)
    {
      if (Tga_head.ColorMapEntrySize>=24)
      {
        buffer=kos_GetMemory(width*height*3);
        int bpp=Tga_head.ColorMapEntrySize/8;
        for(y=height-1;y!=-1;y--)
        {
          for(x=0;x<width;x++)
          {
            r=*(pImg); pImg++;
            cBuffer=buffer+(abs(sm-y)*width*3)+x*3;
            *(cBuffer+0)=(Byte)(pPal[r*bpp+1]);
            *(cBuffer+1)=(Byte)(pPal[r*bpp+2]);
            *(cBuffer+2)=(Byte)(pPal[r*bpp+3]);
          }
        }
        state=0;
      }
    }
    // ЦВетное изображение без сжатия и палитры
    else if (Tga_head.ImageTypeCode==2)
    {
      switch (Tga_head.ImagePixelSize)
      {
        case 32:
        case 24:
          buffer=kos_GetMemory(width*height*3);
          for(y=height-1;y!=-1;y--)
          {
            for(x=0;x<width;x++)
            {
              cBuffer=buffer+(abs(sm-y)*width*3)+x*3;
              *(cBuffer+0)=*(pImg+0); 
              *(cBuffer+1)=*(pImg+1); 
              *(cBuffer+2)=*(pImg+2);
              pImg=pImg+Tga_head.ImagePixelSize/8;
            }
          }
          state=0;
        break;
        case 16:
          buffer=kos_GetMemory(width*height*3);
          for(y=height-1;y!=-1;y--)
          {
            for(x=0;x<width;x++)
            {
              cBuffer=buffer+(abs(sm-y)*width*3)+x*3;
              *(cBuffer+0)=(Byte)((*(Word*)(pImg)) & 31)<<3;
              *(cBuffer+1)=(Byte)((*(Word*)(pImg)>>5) & 31)<<3;
              *(cBuffer+2)=(Byte)((*(Word*)(pImg)>>10) & 31)<<3;       
              pImg=pImg+2;
            }
          }
          state=0;
        break;
      }
    }
    // Монохромное изображение
    else if (Tga_head.ImageTypeCode==3)
    {
      switch (Tga_head.ImagePixelSize)
      {
        case 8:
          buffer=kos_GetMemory(width*height*3);
          for(y=height-1;y!=-1;y--)
          {
            for(x=0;x<width;x++)
            {
              cBuffer=buffer+(abs(sm-y)*width*3)+x*3;
              *(cBuffer+0)=*(pImg); 
              *(cBuffer+1)=*(pImg); 
              *(cBuffer+2)=*(pImg);
              pImg++;
            }
          }
          state=0;
        break;
      }
    }
  return state;
}