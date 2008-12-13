#include "..\kosSyst.h"
#include "pcx.h"

int PCXFile::LoadPCXFile(Byte* filebuff, Dword filesize)
{
  memcpy((Byte*)&Pcx_head,(Byte*)filebuff,sizeof(PCXHeader));

  int state=0;
  if (Pcx_head.bManufacturer==0x0a && Pcx_head.bVersion==0x05)
  {
    width=Pcx_head.dwWidth-Pcx_head.dwX+1;
    height=Pcx_head.dwHeight-Pcx_head.dwY+1;
    buffer=kos_GetMemory(width*height*3);
    
    Byte* pPal=filebuff+filesize-768;
    Byte* pImg=filebuff+128;
    Byte* cBuffer=(Byte*)buffer;
    if (Pcx_head.bNPlanes==1)
    {
      /* 8-bit decoding */
      Dword y,i;
      Byte  cur_byte,counter;
      Dword cWidth;
     
      for(y=0;y<height;y++)
      {
        cWidth=width;
        while(cWidth!=0)
        {
          cur_byte=*(pImg); pImg++;
          counter=1;
          if (cur_byte>=192)
          {
            counter=(cur_byte & 0x3F);
            cur_byte=*(pImg); pImg++;
          }
          for(i=0;i<counter;i++)
          {
            *(cBuffer+0)=(pPal[cur_byte*3+0]);
            *(cBuffer+1)=(pPal[cur_byte*3+1]);
            *(cBuffer+2)=(pPal[cur_byte*3+2]);
            cBuffer=cBuffer+3;
            cWidth--;
          }
        }
      }
    } else if (Pcx_head.bNPlanes==3) {
      /* 24-bit decoding */
      state=1;
    }
  } else {
    state=1;
  }
  return state;
}