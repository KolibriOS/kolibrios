format MZ 
heap 0 
stack 800h 
entry main:start 

segment main use16 

use16 
start: 

 mov ax,_data 
 mov ds, ax 
 mov es, ax 

 xor eax, eax 
 mov ax,ds 

 shl eax, 4 

 mov bx, gdt 
 add ebx, eax ; ebx - линейный адрес gdt 

 mov word [gdtPtr], 2 * 8 -1 ; предел gdt 2 дескриптора = 0x000f 
 mov dword [gdtPtr + 2], ebx 

 lgdt pword [gdtPtr] 

 in al, 0x70 ;запрещаем NMI 
 mov bl, al 
 or al, 0x80 
 out 70h , al 
 in al, 0x71 ;некоторые RTC после записи байта в порт 0х70 
             ;ожидают обращения к порту 0x71 

 cli 

 mov eax, cr0 
 or al, 01b 
 mov cr0, eax 

 mov dx, 0x08 
 mov gs, dx ;gs - глобальный сегмент с базой 0 и пределом 0xFFFFFFFF 
 mov fs, dx

 and al, 0xFE 
 mov cr0, eax 

 sti 

 mov al, bl 
 out 70h , al 
 in al, 71h 

 mov dx,92h 
 in al,dx 
 or al,2 
 out dx,al 

;теперь можно получить доступ ко всей памяти 
;например 
;mov eax, [gs:0xFFFFFFF0] ; 

 mov ah,4ch ;завершение ДОС программы 
 int 21h 

segment _data use16 

 align 16 

 gdt dw 0, 0, 0, 0 ;0 
 _flat dw 0ffffh,0,0F200h,008fh ;08 сегмент данных DPL = 3 
 ;база 0, предел 0xFFFFFFFF 

 gdtPtr dq ? 