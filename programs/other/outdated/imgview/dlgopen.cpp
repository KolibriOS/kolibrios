#include "kosSyst.h"

char* DialogOpenFile(void (*draw)())
{
  sProcessInfo proc;
  kosFileInfo fi;
  char param[6];
  int i;
  Dword pID,msg_num=0;
  char path[1040];
  Byte* pPath=(Byte*)&path;
  
  //Параметры запуска X-TREE
  kos_ProcessInfo(&proc,-1);
  pID=proc.processInfo.PID;
  for(i=3;i!=-1;i--)
  {
    param[i]=(char)('0'+(pID % 10));
    pID=pID/10;
  }
  param[4]=(char)0x20;
  param[5]='O';
  param[6]=(char)0;
  //IPC
  ((Dword*)pPath)[0] = 0;
  ((Dword*)pPath)[1] = 8;
  
  Dword retval;
  __asm
  {
    mov  eax,60
    mov  ebx,1
    mov  ecx,pPath
    mov  edx,1040
    int  40h
  }

  sprintf(fi.fileURL,"/sys/sysxtree");
  fi.rwMode = 7;
  fi.OffsetLow = 0;
  fi.OffsetHigh = (Dword)param;
  fi.dataCount = 0;
  fi.bufferPtr = 0;
  int dlg_pID=kos_FileSystemAccess(&fi);
  if (dlg_pID<=0) return 0;
  
  //kos_SetMaskForEvents(0x47);
  kos_SetMaskForEvents(0x67);
  //draw();
  
  Dword btnID;
  Byte keyCode;
  
  for(;;)
  {
get_next_event:
    //События
    switch (kos_WaitForEvent(50))
    {
      case 1:
        draw();
      break;    
      case 2:
        kos_GetKey(keyCode);
      break;
      case 3:
        kos_GetButtonID(btnID);
      break;
      case 7:
        if (msg_num==0)
        {
          ((Dword*)pPath)[0] = 0;
          ((Dword*)pPath)[1] = 8;
          ((Dword*)pPath)[2] = 0;
          ((Dword*)pPath)[3] = 0;
          ((Dword*)pPath)[4] = 0;
          msg_num=1;
          draw();
        } else {
          return (char*)pPath+16;
        } 
      break;
      default:
        //Жив ли еще X-TREE?
        if (msg_num)
        {
          int proc_count=kos_ProcessInfo(&proc,-1);
          for(i=1;i<=proc_count;i++)
          {
            kos_ProcessInfo(&proc,i);
            if (proc.processInfo.PID==dlg_pID)
            {
              if (proc.processInfo.slot_state==9) 
              {
                return 0; 
              } else {
                goto get_next_event;
              }
            }
          }
          return 0;
        }
      break;
    }
  }
  return 0;
}