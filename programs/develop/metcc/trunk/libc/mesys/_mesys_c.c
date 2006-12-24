#include <stdio.h>
#include <mesys.h>

typedef struct{
  dword subproc  ;//* +0: dword: номер подфункции
  int offset_l   ;//* +4: dword: смещение в файле
  int offset_h   ;//* +8: dword: старший dword смещения (должен быть 0) или поле флагов
  int   size     ;//* +12 = +0xC: dword: размер
  void* data     ;//* +16 = +0x10: dword: указатель на данные
  char  letter   ;//* +20 = +0x14: n db: ASCIIZ-строка с именем файла
  char* name     ;//  или
                  //* +20 = +0x14: db 0
                  //* +21 = +0x15: dd указатель на ASCIIZ-строку с именем файла
} mesys_file;



int  stdcall _msys_read_file(char* filename, int fileoffset,int
                             size,void* data, int* filesize)
{
   mesys_file file;
   int res;

   file.subproc = 0;
   file.letter = 0;
   file.offset_h = 0;
   file.offset_l = fileoffset;
   file.size = size;
   file.name = filename;
   file.data = data;
   
   asm("movl    $70,%eax"); 
   asm("movl    file,%ebx"); 
   asm("int     $0x40");
   asm("movl    %eax, res");
   asm("movl    %ebx, size");
   
   if(res==0)
   {
      if(filesize) filesize = size;
      return 0;
   }else return res;
}

int  stdcall _msys_write_file(char* filename,int fileoffset, int size, void* data)
{
   mesys_file file;

   file.subproc = 3;
   file.letter = 0;
   file.offset_h = 0;
   file.offset_l = fileoffset;
   file.size = size;
   file.name = filename;
   file.data = data;
   
   asm("movl    $70,%eax"); 
   asm("movl    file,%ebx"); 
   asm("int     $0x40");
}

void stdcall _msys_run_program(char* filename,char* parameters)
{
   mesys_file file;

   file.subproc = 7;
   file.letter = 0;
   file.offset_h = parameters;
   file.offset_l = 0;
   file.size = 0;
   file.name = filename;
   file.data = 0;
   
   asm("movl    $70,%eax"); 
   asm("movl    file,%ebx"); 
   asm("int     $0x40");
}

int stdcall _msys_create_file(char* filename)
{
   mesys_file file;

   file.subproc = 2;
   file.letter = 0;
   file.offset_h = 0;
   file.offset_l = 0;
   file.size = 0;
   file.name = filename;
   file.data = (dword)&file;
   
   asm("movl    $70,%eax"); 
   asm("movl    file,%ebx"); 
   asm("int     $0x40");
/* asm("movl    %eax, 0(res)");
   asm("movl    %ebx, 0(size)");
   return res;*/
}