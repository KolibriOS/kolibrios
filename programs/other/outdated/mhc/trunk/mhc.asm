;
; MHC archiver for MenuetOS - very fast compression tool
;
; version 0.09
;
; Written by Nikita Lesnikov (nlo_one@mail.ru, Republic of Belarus, Sluck)
;

;==============================================================================

;
; Brief file format description:
;
;                  +-----------+------------------------+
;  File structure: | Method ID | Compressed data        |
;                  +-----------+------------------------+
;
;  Methods list:
;
;    0. LZP (order-2 specified specially for *.ASM,*.RAW and MeOS executables)
;
;  New methods can be easily added without loss of compatibility
; with older versions
;

;==============================================================================

; SYSTEM HEADER

use32

  org 0x0
  db "MENUET01"
  dd 0x01
  dd ENTRANCE
  dd MHC_END
  dd 0x300000  ; 3 megs of memory needed
  dd 0x2FF000
  dd 0x0
  dd 0x0

include 'lang.inc'
include '..\..\..\macros.inc'
; CODE AREA

ENTRANCE:

; ======== user interface =========


 call draw_window             ; draw the window

 still:

 mov eax,10                   ; wait for event
 mcall

 cmp eax,1                    ; redraw?
 jnz no_redraw
 call draw_window
 no_redraw:

 cmp eax,2                    ; key pressed?
 jz key

 cmp eax,3                    ; button pressed?
 jz button

 jmp still

 ; Key handler

 key:
 mov eax,2   ; read it
 mcall
 shr eax,8

 cmp byte [editstate],0
 jz  still

 cmp al,8    ; backspace
 jnz no_bksp
 cmp byte [editpos],0
 jz  no_del_last
 dec byte [editpos]
 xor ebx,ebx
 mov bl,byte [editpos]
 add ebx,cmfile
 cmp byte [editstate],2
 jnz no_add_base_1
 add ebx,12
 no_add_base_1:
 mov byte [ebx],32
 no_del_last:
 call draw_info
 jmp still
 no_bksp:

 cmp al,13            ; enter
 jnz no_enter
 mov byte [editstate],0
 call draw_info
 jmp still
 no_enter:

 cmp eax,dword 31
 jbe no_lit
 cmp eax,dword 95
 jb  capital
 sub eax,32
 capital:
 xor ebx,ebx
 mov bl,byte [editpos]
 add ebx,cmfile
 cmp byte [editstate],2
 jnz no_add_base_2
 add ebx,12
 no_add_base_2:
 mov byte [ebx],al
 inc byte [editpos]
 cmp byte [editpos],12
 jnz no_null_state
 mov byte [editstate],0
 no_null_state:
 call draw_info
 no_lit:

 jmp still

 ; Button handler

 button:

 mov eax,17
 mcall

 cmp ah,1
 jnz no_quit
 mov eax,-1
 mcall
 no_quit:

 cmp ah,4
 jnz nofirst
 cld
 mov byte [editstate],1
 mov edi,cmfile
 mov eax,0x20202020
 mov ecx,3
 rep stosd
 mov byte [editpos],0
 mov byte [msgid],0
 call draw_info
 nofirst:

 cmp ah,5
 jnz nosecond
 cld
 mov byte [editstate],2
 mov edi,iofile
 mov eax,0x20202020
 mov ecx,3
 rep stosd
 mov byte [editpos],0
 mov byte [msgid],0
 call draw_info
 nosecond:

 cmp ah,2
 jnz no_compress
 call compress
 no_compress:

 cmp ah,3
 jnz no_decompress
 call decompress
 no_decompress:

 cmp ah,6
 jnz no_delete_io
 pusha
 mov eax,32
 mov ebx,iofile
 mcall
 popa
 no_delete_io:

 cmp ah,7
 jnz  no_delete_archive
 pusha
 mov eax,32
 mov ebx,cmfile
 mcall
 popa
 no_delete_archive:

 jmp still

 ; WINDOW DRAW

 draw_window:

 mov eax,12  ; Start redrawing
 mov ebx,1
 mcall

 xor eax,eax           ; Define window
 mov ebx,100*65536+240
 mov ecx,100*65536+130
 mov edx,0x04AAAAAA
 mov esi,0x80777777
 mov edi,0x00777777
 mcall

 mov eax,4              ; Draw all needed texts
 mov ebx,8*65536+8
 mov ecx,0x00FFFFFF
 mov edx,title
 mov esi,arclab-title
 mcall

 xor ecx,ecx
 mov edx,arclab
 mov esi,unplab-arclab
 add ebx,10*65536+28
 mcall

 mov edx,unplab
 mov esi,fin_text-unplab
 add ebx,18
 mcall

 pusha

; mov eax,8            ; Buttons
; mov ebx,222*65536+10
; mov ecx,6*65536+10
; mov edx,1
; mov esi,0x555555
; mcall

 mov eax,8
 mov ebx,15*65536+100
 mov ecx,70*65536+13
 mov edx,2
 mcall

 inc edx
 add ebx,110*65536
 mcall

 inc edx
 mov ebx,214*65536+11
 mov ecx,33*65536+11
 mcall

 inc edx
 add ecx,18*65536
 mcall

 inc edx
 mov ebx,15*65536+100
 mov ecx,86*65536+13
 mcall

 inc edx
 add ebx,110*65536
 mcall

 popa

 mov ecx,0x00FFFFFF
 mov edx,keylab
 mov esi,dellab-keylab
 add ebx,19
 mcall

 mov edx,dellab
 mov esi,title-dellab
 add ebx,16
 mcall

 call draw_info

 mov eax,12          ; Finish redrawing
 mov ebx,2
 mcall

 ret

 draw_info:          ; Draw filenames and compressor state

 activecolor equ 0x00112299

 pusha ; Save registers

 mov eax,13               ; Clean draw area
 mov ebx,127*65536+85
 mov ecx,33*65536+33
 mov edx,0x00AAAAAA
 mcall

 mov eax,4 ; Draw filenames
 mov ebx,134*65536+36
 mov edx,cmfile
 xor ecx,ecx
 mov esi,12
 cmp byte [editstate],1
 jnz no_active_1
 mov ecx,activecolor
 no_active_1:
 mcall
 xor ecx,ecx
 cmp byte [editstate],2
 jnz no_active_2
 mov ecx,activecolor
 no_active_2:
 add ebx,18
 add edx,12
 mcall

 mov eax,13             ; Clean info area
 mov ebx,14*65536+210
 mov ecx,107*65536+14
 mov edx,0x00AAAAAA
 mcall

 cmp byte [msgid],0     ; Draw info string
 jz notype
 mov ebx,16*65536+110
 xor ecx,ecx
 mov esi,16
 mov al, byte [msgid]
 dec al
 shl al,4
 xor ah,ah
 xor edx,edx
 mov dx,ax
 add edx,msgtable
 mov eax,4
 mcall
 notype:

 popa ; Restore registers

 ret

 ; interface data

 if lang eq de
 keylab db "    PACKEN           ENTPACKEN"
 dellab db "   LOESCHE I/O      LOESCHE *.MHC"
 title  db "MHC 0.09"
 arclab db "GEOACJTE DATEI:"
 unplab db "EIN/AUSGABE DATEI:"
 fin_text:

 cmfile db "FILENAME.MHC"
 iofile db "FILENAME.XYZ"

 msgtable:
 db "PACKE...        "
 db "ENTPACKE...     "
 db "KEIN I/O!       "
 db "KEINE *.MHC!    "
 db "FALSCHE METHODe!"

 else
 keylab db "    COMPRESS         DECOMPRESS"
 dellab db "   DELETE I/O       DELETE *.MHC"
 title  db "MHC 0.09"
 arclab db "COMPRESSED FILE:"
 unplab db "INPUT/OUTPUT FILE:"
 fin_text:

 cmfile db "FILENAME.MHC"
 iofile db "FILENAME.XYZ"

 msgtable:
 db "COMPRESSING...  "
 db "DECOMPRESSING..."
 db "I/O NOT FOUND!  "
 db "*.MHC NOT FOUND!"
 db "INVALID METHOD! "

 end if


 editstate db 0
 editpos db 0
 msgid db 0


; ======== compression/decompression engine ========

; Adresses declaration

 hashtable equ  MHC_END
 ifile     equ  hashtable+65536*4
 ofile     equ  ifile+1000000

 compress:   ; File compression

 call fill_filebufs

 mov eax,6
 mov ebx,iofile
 xor ecx,ecx
 mov edx,ecx
 not edx
 mov esi,ifile
 mcall

 cmp eax,0xFFFFFFFF
 jnz  compress_filefound              ; i/o file not found
 mov byte [msgid],3
 call draw_info
 ret

 compress_filefound:

 mov byte [msgid],1
 call draw_info

 jmp lzp_compress                    ; compress with order-2 LZP
 compress_dumpdata:

 push edx

 mov eax,32
 mov ebx,cmfile
 mcall

 mov eax,33
 pop edx
 mov ebx,cmfile
 mov ecx,ofile
 xor esi,esi
 mcall

 mov byte [msgid],0
 call draw_info

 ret


 decompress: ; File decompression

 call fill_filebufs

 mov  eax,6
 mov ebx,cmfile
 xor ecx,ecx
 mov edx,ecx
 not edx
 mov esi,ofile
 mcall

 cmp eax,0xFFFFFFFF
 jnz  decompress_filefound              ; *.mhc file not found
 mov byte [msgid],4
 call draw_info
 ret

 decompress_filefound:

 cmp byte [ofile],0                     ; Invalid method!
 jz  right_method
 mov byte [msgid],5
 call draw_info
 ret

 right_method:
 mov byte [msgid],2
 call draw_info

 jmp lzp_decompress
 decompress_dumpdata:

 push edx

 mov eax,32
 mov ebx,iofile
 mcall

 mov eax,33
 pop edx
 mov ebx,iofile
 mov ecx,ifile
 xor esi,esi
 mcall

 mov byte [msgid],0
 call draw_info

 ret

 fill_filebufs:             ; Fill filebufs with garbage to simplify matching
 pusha
 cld
 mov eax,0xF7D9A03F         ; <- "magic number" :) just garbage...
 mov ecx,2000000/4
 mov edi,ifile
 rep stosd
 popa
 ret

; ==== algorithms section ====

; Method 0: LZP compression algorithm

 lzp_compress:           ; EDX - how much bytes to dump

 cld                     ; clear direction flag

 mov esi,ifile           ; init pointers
 mov edi,ofile

 push eax                ; write header: ID0+4bfilesize => total 5 bytes
 xor eax,eax
 stosb
 pop eax
 stosd

 pusha                   ; fill hash table
 mov eax,ifile
 mov edi,hashtable
 mov ecx,65536
 rep stosd
 popa

 add eax,esi              ; calculate endpointer
 mov dword [endpointer],eax

 movsw                    ; copy three bytes
 movsb

 mov dword [controlp],edi
 inc edi

 mov byte [controld],0
 mov byte [controlb],0

 c_loop:
 cmp dword [endpointer],esi  ; check end of file
 ja  c_loop_ok
 jmp finish_c_loop
 c_loop_ok:

 call chash
 call compare
 jz   two_match_c

 lodsb
 mov byte [literal],al
 call chash
 call compare
 jz   lit_match_c

 mov  al,0
 call putbit
 mov  al,byte [literal]
 stosb
 movsb
 jmp  end_c_loop

 lit_match_c:
 mov al,1
 call putbit
 mov al,0
 call putbit
 mov al,byte [literal]
 stosb
 jmp encode_match

 two_match_c:
 mov al,1
 call putbit
 call putbit

 encode_match:
 call incpos
 call compare
 jz one_c
 mov al,0
 call putbit
 jmp end_c_loop
 one_c:

 call incpos
 mov  al,1
 call putbit

 call compare
 jnz ec1
 call incpos
 call compare
 jnz ec2
 call incpos
 call compare
 jnz ec3
 call incpos
 mov al,1
 call putbit
 call putbit
 call compare
 jnz ec4
 call incpos
 call compare
 jnz ec5
 call incpos
 call compare
 jnz ec6
 call incpos
 call compare
 jnz ec7
 call incpos
 call compare
 jnz ec8
 call incpos
 call compare
 jnz ec9
 call incpos
 call compare
 jnz ec10
 call incpos

 mov al,1
 call putbit
 call putbit
 call putbit
 xor  ecx,ecx

 match_loop_c:
 cmp  esi,dword [endpointer]
 jae   out_match_loop_c
 call compare
 jnz  out_match_loop_c
 inc  ecx
 call incpos
 jmp  match_loop_c
 out_match_loop_c:

 mov al,0xFF
 out_lg:
 cmp ecx,255
 jb  out_lg_out
 stosb
 sub ecx,255
 jmp out_lg
 out_lg_out:
 mov al,cl
 stosb
 jmp end_c_loop

 ec10:
 mov al,1
 call putbit
 call putbit
 mov al,0
 call putbit
 jmp end_c_loop

 ec9:
 mov al,1
 call putbit
 mov al,0
 call putbit
 mov al,1
 call putbit
 jmp end_c_loop

 ec8:
 mov al,1
 call putbit
 mov al,0
 call putbit
 call putbit
 jmp end_c_loop

 ec7:
 mov al,0
 call putbit
 mov al,1
 call putbit
 call putbit
 jmp end_c_loop

 ec6:
 mov al,0
 call putbit
 mov al,1
 call putbit
 mov al,0
 call putbit
 jmp end_c_loop

 ec5:
 mov al,0
 call putbit
 call putbit
 mov al,1
 call putbit
 jmp end_c_loop

 ec4:
 mov al,0
 call putbit
 call putbit
 call putbit
 jmp end_c_loop

 ec3:
 mov al,1
 call putbit
 mov al,0
 call putbit
 jmp end_c_loop

 ec2:
 mov al,0
 call putbit
 mov al,1
 call putbit
 jmp end_c_loop

 ec1:
 mov al,0
 call putbit
 call putbit

 end_c_loop:
 jmp c_loop

 finish_c_loop:

 mov eax,dword [controlp] ; store last tagbyte
 mov bl,byte [controld]
 mov [eax], byte bl

 sub edi,ofile ; calculate dump size
 mov edx,edi

 jmp compress_dumpdata

; LZP decompression algorithm

 lzp_decompress:                        ; EDX - how much bytes to dump

 cld

 mov edi,ifile
 mov esi,ofile+1

 pusha                   ; fill hash table
 mov eax,ifile
 mov edi,hashtable
 mov ecx,65536
 rep stosd
 popa

 lodsd

 mov ebx,edi
 add ebx,eax
 mov dword [endpointer],ebx

 movsw
 movsb

 lodsb
 mov byte [controld],al
 mov byte [controlb],0

 d_loop:
 cmp dword [endpointer],edi
 ja d_loop_ok
 jmp finish_d_loop
 d_loop_ok:

 call getbit
 cmp  al,0
 jnz  match_d
 call dhash
 movsb
 call dhash
 movsb
 jmp end_d_loop

 match_d:

 call getbit
 cmp  al,0
 jnz  no_literal_before_match
 call dhash
 movsb
 no_literal_before_match:

 call dhash
 mov ecx,1
 call copymatch

 call getbit
 cmp  al,0
 jz   end_d_loop
 mov  ecx,1
 call copymatch
 call getbit
 cmp  al,0
 jz   dc2
 mov  ecx,2
 call copymatch
 call getbit
 cmp  al,0
 jz   end_d_loop
 mov  ecx,1
 call copymatch
 call getbit
 cmp  al,0
 jz   dc4
 mov  ecx,4
 call copymatch
 call getbit
 cmp  al,0
 jz   dc5
 call getbit
 cmp  al,0
 jz   dc6
 mov  ecx,3
 call copymatch

 do:
 lodsb
 xor  ecx,ecx
 mov  cl,al
 call copymatch
 cmp  al,0xFF
 jnz  end_do
 jmp do
 end_do:
 jmp end_d_loop

 dc6:
 mov ecx,2
 call copymatch
 jmp  end_d_loop

 dc5:
 call getbit
 cmp  al,0
 jz   ndc5
 mov  ecx,1
 call copymatch
 ndc5:
 jmp  end_d_loop

 dc4:
 call getbit
 cmp  al,0
 jz   ndc4
 call getbit
 mov  ecx,3
 cmp  al,1
 jz   ndcc4
 dec  ecx
 ndcc4:
 call copymatch
 jmp  end_d_loop
 ndc4:
 call getbit
 cmp  al,0
 jz   ndccc4
 mov  ecx,1
 call copymatch
 ndccc4:
 jmp  end_d_loop

 dc2:
 call getbit
 cmp al,0
 jz  ndc2
 mov ecx,1
 call copymatch
 ndc2:

 end_d_loop:
 jmp d_loop
 finish_d_loop:

 mov edx, dword [ofile+1]

 jmp decompress_dumpdata

; LZP subroutines

 putbit:                  ; bit -> byte tag, AL holds bit for output
 pusha
 mov cl,byte [controlb]
 shl al,cl
 mov bl,byte [controld]
 or  bl,al
 mov byte [controld],bl
 inc cl
 cmp cl,8
 jnz just_increment
 mov byte [controlb],0
 mov byte [controld],0
 push edi
 mov  edi, dword [controlp]
 mov  al,bl
 stosb
 pop  edi
 mov dword [controlp],edi
 popa
 inc edi
 ret
 just_increment:
 mov byte [controlb],cl
 popa
 ret

 getbit:                       ; tag byte -> bit, AL holds input
 push ecx
 mov al,byte [controld]
 mov cl,byte [controlb]
 shr al,cl
 and al,1
 inc cl
 cmp cl,8
 jnz just_increment_d
 mov byte [controlb],0
 push eax
 lodsb
 mov byte [controld],al
 pop  eax
 pop  ecx
 ret
 just_increment_d:
 mov byte [controlb],cl
 pop ecx
 ret

 chash:                        ; calculate hash -> mp -> fill position
 pusha
 xor  eax,eax
 mov  al, byte [esi-1]
 mov  ah, byte [esi-2]
 shl  eax,2
 add  eax,hashtable
 mov  edx,dword [eax]
 mov  dword [mp],edx
 mov  dword [eax],esi
 popa
 ret

 dhash:                        ; calculate hash -> mp -> fill position
 pusha
 xor  eax,eax
 mov  al, byte [edi-1]
 mov  ah, byte [edi-2]
 shl  eax,2
 add  eax,hashtable
 mov  edx,dword [eax]
 mov  dword [mp],edx
 mov  dword [eax],edi
 popa
 ret

 copymatch:                    ; ECX bytes from [mp] to [rp]
 push esi
 mov  esi,dword [mp]
 rep  movsb
 mov  dword [mp],esi
 pop  esi
 ret

 compare:                      ; compare [mp] with [cpos]
 push edi
 push esi
 mov  edi,dword [mp]
 cmpsb
 pop  esi
 pop  edi
 ret

 incpos:
 inc  dword [mp]
 inc  esi
 ret


; LZP algorithm data

 endpointer     dd      0
 controlp       dd      0
 controlb       db      0
 controld       db      0
 mp  dd 0
 literal        db      0

MHC_END: ; the end... - Nikita Lesnikov (nlo_one)
