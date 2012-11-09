include "MACROS.INC"

initipc:
 mov eax,9
 mov ebx,prc
 mov ecx,-1
 int 0x40

 mov ecx,eax
loop1:
 push ecx

 mov eax,9
 mov ebx,prc
 int 0x40

 cmp word [prc+10],'CM'
 jne no_cmd
 cmp byte [prc+12],'D'
 jne no_cmd

 mov ebx,[prc+30]
 mov dword [cmdpid],ebx

 mov dword [cmdnumb],ecx

no_cmd:
 pop ecx
 loop loop1

 cmp dword [cmdpid],0
 jne no_exit

 jmp exit

no_exit:
 mov eax,60
 mov ebx,2
 mov ecx,dword [cmdpid]
 mov edx,printf
 mov esi,4
 int 0x40

 call initcmd

waitcmdinit:
 mov eax,40
 mov ebx,01000000b
 int 0x40

 mov eax,23
 mov ebx,100
 int 0x40

 cmp eax,7
 je cmd_ok

 jmp exit

cmd_ok:
 cmp byte [ipcb+16],'.'
 jne exit

 mov eax,18
 mov ebx,3
 mov ecx,dword [cmdnumb]
 int 0x40

 ret

pause1:
 mov eax,5
 mov ebx,1
 int 0x40
 ret

exit:
 mov eax,-1
 int 0x40

cls:
 mov eax,60
 mov ebx,2
 mov ecx,dword [cmdpid]
 mov edx,ipccls
 mov esi,4
 int 0x40

 call pause1

 ret

print:
 mov ecx,84
loopprt:
 mov edi,stripc
 add edi,ecx
 mov esi,fill_symbol
 movsb

 loop loopprt

 cld
 mov ecx,4
 mov edi,stripc
 mov esi,printf
 rep movsb

 cld
 mov edx,79
 sub edx,eax
 mov ecx,79
 sub ecx,edx
 mov edi,stripc+4
 mov esi,ebx
 rep movsb

 mov eax,60
 mov ebx,2
 mov ecx,dword [cmdpid]
 mov edx,stripc
 mov esi,84
 int 0x40

 call pause1

 ret

eol:
 mov eax,60
 mov ebx,2
 mov ecx,dword [cmdpid]
 mov edx,ipceol
 mov esi,4
 int 0x40

 call pause1

 ret

initcmd:
 mov eax,60
 mov ebx,2
 mov ecx,dword [cmdpid]
 mov edx,ipckey
 mov esi,4
 int 0x40

 mov eax,60
 mov ebx,1
 mov ecx,ipcb
 mov edx,28
 int 0x40

 cld
 mov ecx,28
 mov edi,ipcb
 mov esi,ipcc
 rep movsb

 ret

getkey:
 call initcmd

waitagain:
 mov eax,40
 mov ebx,01000000b
 int 0x40

 mov eax,10
 int 0x40

 cmp eax,7
 jne waitagain

 mov edi,key
 mov esi,ipcb+16
 movsb

 ret

endipc:
 mov eax,60
 mov ebx,2
 mov ecx,dword [cmdpid]
 mov edx,ipcend
 mov esi,4
 int 0x40

 jmp exit

cmdpid dd 0
cmdnumb dd 0

printf db '~ppp'
ipceol db '~lll'
ipcend db '~eee'
ipccls db '~ccc'
ipckey db '~kkk'

key db 0

ipcb:
 db 0
 db 0,0,0
 dd 8
times 20 db 0

ipcc:
 db 0
 db 0,0,0
 dd 8
times 20 db 0

stripc: times 84 db 0

fill_symbol db 0

prc: times 52 db 0

