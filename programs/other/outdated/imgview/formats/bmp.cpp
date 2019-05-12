#include "..\kosSyst.h"
#include "bmp.h"

int BMPFile::LoadBMPFile(Byte* filebuff, Dword filesize)
{
  Dword offset;

  memcpy((Byte*)&Bmp_head,(Byte*)filebuff,sizeof(tagBITMAPFILEHEADER));
  memcpy((Byte*)&Info_head,(Byte*)filebuff+14,sizeof(tagBITMAPINFOHEADER));

  width=Info_head.biWidth;
  height=Info_head.biHeight;
  offset=(Dword)Bmp_head.bfOffbits;

  int state=0;
  if (Bmp_head.bfType==0x4d42 && !Info_head.biCompression)
  {
    Byte* cBuffer;
    Byte* pImg; 
    Byte* pPal;
    Dword x,y;
    Byte  r;
    Dword s,p;
    Dword cWidth;
    int i;
    
    buffer=kos_GetMemory(width*height*3);
    pImg=filebuff+offset;
    pPal=filebuff+53;
    
    int align_bytes = (4 - ((width * 3) % 4)) % 4;
    Dword bpp = Info_head.biBitCount;
    
    switch(Info_head.biBitCount)
    {
      /* 16,24,32-bit decoding */
      case 32:
      case 24:
      case 16:
        for(y=height-1;y!=-1;y--)
        {
          for(x=0;x<width;x++)
          {
            cBuffer=buffer+(y*width*3)+x*3;
            if (Info_head.biBitCount==16) 
            {
              *(cBuffer+0)=(Byte)((*(Word*)(pImg)) & 31)<<3;
              *(cBuffer+1)=(Byte)((*(Word*)(pImg)>>5) & 31)<<3;
              *(cBuffer+2)=(Byte)((*(Word*)(pImg)>>10) & 31)<<3;
            } else {
              *(cBuffer+0)=*(pImg+0);
              *(cBuffer+1)=*(pImg+1);
              *(cBuffer+2)=*(pImg+2);
            }
            pImg=pImg+Info_head.biBitCount/8;
          }
          pImg=pImg+align_bytes;
        }
      break;
      /* 8-bit decoding */
      case 8:
        for(y=height-1;y!=-1;y--)
        {
          for(x=0;x<width;x++)
          {
            r=*(pImg); pImg++;
            cBuffer=buffer+(y*width*3)+x*3;
            *(cBuffer+0)=(Byte)(pPal[r*4+1]);
            *(cBuffer+1)=(Byte)(pPal[r*4+2]);
            *(cBuffer+2)=(Byte)(pPal[r*4+3]);
          }
        }
      break;
      /* 1,4-bit decode */
      case 4:
      case 1:
        for(y=height-1;y!=-1;y--)
        {
          x=0;
          while(x<width-1)
          {
            s=*(Dword*)(pImg); 
            pImg=pImg+4;
            __asm
            {
              mov  eax,s
              bswap eax
              mov  s,eax
            }
            for(i=0;i<32/bpp;i++)
            {
              if (x>=width) break;
              __asm
              {
                mov  eax,s
                mov  ecx,bpp
                rol  eax,cl
                mov  s,eax
                mov  ebx,1
                shl  ebx,cl
                dec  ebx
                and  eax,ebx
                mov  p,eax
             }
              cBuffer=buffer+(y*width*3)+x*3;
              *(cBuffer+0)=(Byte)(pPal[p*4+1]);
              *(cBuffer+1)=(Byte)(pPal[p*4+2]);
              *(cBuffer+2)=(Byte)(pPal[p*4+3]);
              x++;
            }
          }
        }
      break;
      default:
        state=1;
      break;
    }
  } else {
    state=1;
  }
  return state;
}