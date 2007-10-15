format ELF

section '.text' executable

include 'proc32.inc'
public _ksys_get_filesize
public _ksys_readfile
public _ksys_rewritefile
public _ksys_appendtofile

align 4
proc _ksys_get_filesize stdcall, filename:dword

        xor eax,eax
        mov ebx,[filename]
        mov [fileinfo.subproc],dword 5
        mov [fileinfo.offset_l],eax
        mov [fileinfo.offset_h],eax
        mov [fileinfo.size],eax
        mov [fileinfo.data],dword buffer_for_info
        mov [fileinfo.letter],al
        mov [fileinfo.filename],ebx

        mov eax,70
        mov ebx,fileinfo
        int 0x40

        test eax,eax
        jnz error_for_file_size

          mov eax,[buffer_for_info+32] ;file size

        error_for_file_size:

        ret
endp


align 4
proc _ksys_readfile stdcall,filename:dword,position:dword,sizeblock:dword,buffer:dword

        xor eax,eax
        mov ebx,[position]
        mov ecx,[sizeblock]
        mov edx,[buffer]
        mov esi,[filename]
        mov [fileinfo.subproc],eax
        mov [fileinfo.offset_l],ebx
        mov [fileinfo.offset_h],eax
        mov [fileinfo.size],ecx
        mov [fileinfo.data],edx
        mov [fileinfo.letter],al
        mov [fileinfo.filename],esi

        mov eax,70
        mov ebx,fileinfo
        int 0x40

        ret
endp

align 4
proc _ksys_rewritefile stdcall,filename:dword,sizeblock:dword,data_write:dword

        xor eax,eax
        mov ebx,[sizeblock]
        mov ecx,[data_write]
        mov edx,[filename]
        mov [fileinfo.subproc],dword 2
        mov [fileinfo.offset_l],eax
        mov [fileinfo.offset_h],eax
        mov [fileinfo.size],ebx
        mov [fileinfo.data],ecx
        mov [fileinfo.letter],al
        mov [fileinfo.filename],edx

        mov eax,70
        mov ebx,fileinfo
        int 0x40

        ret
endp

align 4
proc _ksys_appendtofile stdcall,filename:dword,pos:dword,sizeblock:dword,data_append:dword

        xor eax,eax
        mov ebx,[pos]
        mov ecx,[sizeblock]
        mov edx,[data_append]
        mov esi,[filename]
        mov [fileinfo.subproc],dword 3
        mov [fileinfo.offset_l],ebx
        mov [fileinfo.offset_h],eax
        mov [fileinfo.size],ecx
        mov [fileinfo.data],edx
        mov [fileinfo.letter],al
        mov [fileinfo.filename],esi

        mov eax,70
        mov ebx,fileinfo
        int 0x40

        ret
endp

struc FILEIO
{
 .subproc          rd 1
 .offset_l         rd 1
 .offset_h         rd 1
 .size             rd 1
 .data             rd 1
 .letter           rb 1
 .filename         rd 1
}

fileinfo           FILEIO
buffer_for_info    rd 11
