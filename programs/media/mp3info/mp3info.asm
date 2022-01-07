; AUTHORS:
; S. Kuzmin, A. Ershov, Madis Kalme 2005
; Sergey Efremenkov, Leency 2018

; CHECK OUT README.TXT!

format binary as ""

    use32
    org    0x0
    db     'MENUET01'            ; 8 byte id
    dd     0x01                  ; header version
    dd     START                 ; start of code
    dd     I_END                 ; size of image
    dd     0x9000                ; memory for app
    dd     0x9000                ; esp
    dd     fileinfo2.path        ; I_Param , I_Icon
    dd     0x0

include "mos_uzit.inc"

TAG1_X = 250     ; coordinates of ID3v1 block
TAG1_Y = 40

BLOCKS_TO_READ  equ     2 ; must be greater than 1
BLOCK_SIZE equ 512 ;сколько байт в блоке

START:

    xor  eax, eax
    mov  [last_err], eax
    mov  [fileinfo2+4], eax      ; start block = 0
    mov  dword [fileinfo2+12], BLOCKS_TO_READ*BLOCK_SIZE
    mcall 70, fileinfo2

result:
        push eax ebx                    ;получаем размер файла в байтах
        mcall 70, fileSizeInfo
	
        cmp eax, 0
        je @f
        mov dword[size], 16384; ebx
        jmp result.sizeEnd
@@:
        mov eax, dword[fileBuf+32]  ; на самом деле там размер 8 байт, а не 4
        mov dword[size], eax
.sizeEnd:
        pop ebx eax
        ;mov     dword [size], 16384; ebx

        ; checking ID3v2 tag
        xor     eax, eax
        mov     [tagv2], eax
        mov     eax, [mp3_file]
        and     eax, 0x00ffffff
        cmp     eax, 'ID3'
        jnz     .no_id3v2
        mov     eax, [mp3_file+3]
        mov     [tagv2], eax

        mov     ecx, 4
        mov     esi, mp3_file+6
        xor     eax, eax
        xor     ebx, ebx
        cld
.size_read:
        lodsb
        shl     ebx, 7
        or      ebx, eax
        loop    .size_read
        add     ebx, 10
        mov     eax, ebx
        shr     eax, 9

        push eax edx
        mov edx, BLOCK_SIZE
        mul edx
        mov dword[fileinfo2+4], eax   ; start block
        pop edx eax

        shl     eax, 9

        mov     ecx, ebx
        add     ecx, 3

        sub     ebx, eax
        mov     edi, ebx
        add     edi, mp3_file
        dec     edi

        mcall 70, fileinfo2

        jmp     .loop
.no_id3v2:

        mov     ecx, 3
        mov     edi, mp3_file-1
.loop:
        inc     edi
        inc     ecx
        cmp     ecx, dword[size]
        ja      .no_frames
        mov     eax, [edi]
        call    Header_Check
        test    eax, eax
        jz      .header_found
        cmp     edi, mp3_file+BLOCKS_TO_READ*512-4
        jb      .loop

        add     dword [fileinfo2+4], (BLOCKS_TO_READ-1)*BLOCK_SIZE

        mcall 70, fileinfo2

        sub     edi, (BLOCKS_TO_READ-1)*512
        jmp     .loop

.no_frames:
        mov     [last_err], err_bad_file
        mov     [last_err_l], err_bad_file_e - err_bad_file
        jmp     reading_end

.header_found:
mov eax, dword [edi]
sub     ecx, 4
mov     [header_at], ecx


call extract_bits

call decode_standard

call decode_layer

call decode_channels

call decode_samplerate

call decode_bitrate

call calculate_time_frame_count

;--------------------------------------------

        mov     eax, [b1s]
        and     eax, 1
        shl     eax, 1                  ; eax = eax * 2
        cmp     byte [shan], 11b        ; if mono
        jz      @f
        inc     eax
@@:
        mov     ebx, xing_offset
        xlatb

        add     edi, eax
        mov     eax, [edi]

        xor     ebx, ebx
        mov     [xing_tag], ebx
        cmp     eax, 'Xing'
        jnz     .no_xing_tag
        mov     esi, edi
        add     esi, 15
        std
        mov     edi, xing_bytes
        mov     ecx, 3*4
        xor     eax, eax

.xing_read:
        lodsb
        mov     [edi], al
        inc     edi
        loop    .xing_read
        cld
        mov     ebx, [xing_tag]
        test    ebx, 1
        jz      .frames_end
        mov     eax, [xing_frames]
        mov     [framecount], eax
        test    [b1s], 1        ; if MPEG 1 eax = eax*2
        jz      @f
        shl     eax, 1
@@:
        mov     ebx, 575
        mul     ebx             ; edx:eax = eax*575
        mov     ebx, [SR]
        div     ebx             ; eax = edx:eax / sample rate

        mov     [time], eax

        ; calculating bitrate
        xor     edx, edx
        mov     ebx, 1000 / 8
        mul     ebx             ; edx:eax = time * 1000
        mov     ebx, eax
        mov     eax, [xing_bytes]
        div     ebx             ; eax = size / time*1000
        mov     [BR], eax


.frames_end:


.no_xing_tag:

xor eax, eax
xor ebx, ebx
xor ecx, ecx
xor edx, edx
;       ID3v1 tag reading
        mov     eax, [size]     ; reading 2 last 512-byte blocks where our
        mov     ebx, eax        ;       tag may be
        shr     eax, 9          ; eax = size of file in full 512-byte blocks
        test    eax, eax        ; if file's length is less then 512 we'll
        jz      @f              ; read the whole file
        dec     eax
@@:
        push eax edx
        mov edx, BLOCK_SIZE
        mul edx
        mov     dword [fileinfo2+4], eax       ; start block
        pop edx eax
        mov     dword [fileinfo2+12], 2*BLOCK_SIZE         ; blocks to read

        shl     eax, 9
        sub     ebx, eax
        add     ebx, mp3_file - 128     ; if tag is present, ebx points to it
        mov     esi, ebx

        xor     eax, eax
        mov     [tag], eax


        mcall   70, fileinfo2

        mov     eax, [esi]      ; checking if tag is present
        and     eax, 0x00ffffff
        cmp     eax, 'TAG'
        jnz      @f
        inc     [tag]
        cld
        mov     ecx, 128 / 4
        mov     edi, tag.data
        rep     movsd
        mov     esi, tag.data
        mov     ecx, 126
        call    Win2Dos
@@:
reading_end:
   call    draw_window                     ; 14.08.05 [


        ; Цикл получения и обработки сообщений
   get_event:
        mov  eax,10
        int  0x40
        dec     eax
        jnz     @f
        call    draw_window
        jmp     get_event
@@:
        dec     eax
        jz      key_on_keyboard
        dec     eax
        jz      click_on_button
        jmp     get_event

   key_on_keyboard:
        mcall   2
        jmp     get_event

   click_on_button:
   exit:
        mcall   17
        ;cmp     ah, 10
        ;jz      other_file

        mov  eax,-1
        int  0x40

        jmp     get_event               ;     ] 14.08.05

           ; Рисование окна
draw_window:
        mcall 12,1
        mov  eax,0              ; function 0 : define and draw window
        mov  ebx,250 shl 16 + 500
        mov  ecx,250 shl 16 + 300
        mov  edx,0x34aabbcc     ; color of work area RRGGBB,8->color gl
		mov  edi, header
        int  0x40

        Text 20,12,0x00000000,choice, choicelen-choice
        Text 110,12,0x00000000,fileinfo2.path, 60

        mov     edx, dword [last_err]
        test    edx, edx
        jz      .no_err
        mov     eax, 4
        mov     ebx, 50*65536+50
        xor     ecx, ecx
        mov     esi, [last_err_l]
        int     0x40
        jmp     draw_end
.no_err:

      Text 20,40,0x00000000,S, Slen-S
      Number 110,40,1*256,1,dword [S1],0x000000;
      Number 122,40,1*256,1,dword [S2],0x000000;

      Text 20,60,0x00000000,L, Llen-L
      Number 110,60,1*256,1,dword [La],0x000000

          Text 20,100,0x00000000,SamR, SamRlen-SamR
          Number 110,100,0*256,5,dword [SR],0x000000

          Text 20,120,0x00000000,BitR, BitRlen-BitR
          Number 110,120,0*256,3,dword [BR],0x000000

        mov     eax, [xing_tag]
        test    eax, eax
        jz      @f
        Text 170,120,0x00000000,vbr, vbr_e - vbr
@@:

          Text 20,140,0x00000000,Sizebyte, Sizebytelen-Sizebyte
      Number 110,140,0*256,8,dword [size],0x000000

          Text 20,160,0x00000000,Timese, Timeselen-Timese
          Number 110,160,0*256,4,dword [time],0x000000

          Text 20,180,0x00000000,frame, framelen-frame
          Number 110,180,0*256,4,dword [frames],0x000000

        Text 20,200,0x00000000,framcount, framcountlen-framcount

        Text 20,220,0x00000000,padding, paddinglen-padding

        cmp [pad], 1
        je res1
        jne res2

        res1:
     Text 75,220,0x00000000,da, dalen-da
         jmp nex
        res2:
         Text 75,220,0x00000000,net, netlen-net
         jmp nex

         nex:

        ; ------------------


        Text 110,220,0x00000000,crci, crcilen-crci

        cmp [crc], 0
        je res3
        jne res4

        res3:
     Text 160,220,0x00000000,da, dalen-da
         jmp ne
        res4:
         Text 160,220,0x00000000,net, netlen-net
         jmp ne

         ne:

         ;--------------------------

          Number 110,200,0*256,6,dword [framecount],0x000000


          Text 20,80,0x00000000,Ka, Kalen-Ka

         cmp [K], 1
         je rez1
         cmp [K], 2
         je rez2
         cmp [K], 3
         je rez3
         cmp [K], 4
         je rez4

         rez1:
         Text 110,80,0x00000000,SC, SClen-SC
         jmp next
         rez2:
         Text 110,80,0x00000000,DC, DClen-DC
          jmp next
         rez3:
         Text 110,80,0x00000000,JOS, JOSlen-JOS
          jmp next
         rez4:
         Text 110,80,0x00000000,Su, Sulen-Su

next:
        Text   20,240,0,header_found, header_found_e - header_found
        Number 160,240,0*256,6,dword [header_at],0x000000  ;;;;;;; HEADER AT



        ; ID3v2

        mcall   4, 250*65536+220, 0,id3v2, id3v2_e - id3v2
        mov     edi, tagv2
        mov     eax, [edi]
        test    eax, eax
        jz      .no_v2
        mcall   4, 281*65536+220,  ,dots, dots_e - dots
        xor     ecx, ecx
        mov     cl, byte [edi]
        mcall   47, 65536*2, , 286*65536+220, 0
        mov     cl, byte [edi+1]
        mcall     ,        , , 304*65536+220,

        jmp     .v2_end
.no_v2:
        Text   300,220,0,net, netlen- net
.v2_end:



        ; ID3v1 info
        ; Writing all field names
        mov     eax, 4                  ; function 4
        words2reg   ebx, TAG1_X, TAG1_Y
        xor     ecx, ecx                ; color
        mov     edx, id3v1
        mov     esi, id3v1_e - id3v1
        int     0x40

        add     ebx, 40
        mov     edx, title ; length is the same, so we don't write it in esi
        int     0x40

        add     ebx, 20
        mov     edx, artist
        inc     esi
        int     0x40

        add     ebx, 20
        mov     edx, album
        dec     esi
        int     0x40

        add     ebx, 20
        mov     edx, year
        dec     esi
        int     0x40

        add     ebx, 20
        mov     edx, genre
        inc     esi
        int     0x40

        add     ebx, 20
        mov     edx, comment
        inc     esi
        inc     esi
        int     0x40

        sub     ebx, 120
        mov     edx, track
        int     0x40

        ; checking if tag is
        mov     eax, dword [tag]
        test    eax, eax
        jz      .no_tag1

        mov     edi, tag.data

        ; writing walues
        words2reg   edx, (TAG1_X+50), (TAG1_Y+20)

        ; track number uses the 30-th byte of comment field
        ; it may be the last char in comment field
        ; so we check if this byte presents a track number
        mov     eax, [edi+125]
        test    al, al
        jnz     .no_track
        test    ah, ah
        jz      .no_track

        mov     ebx, 3*65536
        xor     ecx, ecx
        mov     cl, ah
        xor     esi, esi
        mov     eax, 47
        int     0x40

.no_track:
        mov     ebx, edx
        mov     edx, edi
        mov     eax, 4          ; function 4
        xor     ecx, ecx        ; color

        add     ebx, 20
        add     edx, 3
        mov     esi, 30
        int     0x40            ; title

        add     ebx, 20
        add     edx, esi
        int     0x40            ; artist

        add     ebx, 20
        add     edx, esi
        int     0x40            ; album

        add     ebx, 60
        add     edx, 34
        dec     esi
        int     0x40            ; comment

        sub     ebx, 40
        mov     esi, 4
        sub     edx, esi
        int     0x40            ; year

.no_tag1:
draw_end:
    mcall 12, 2
ret                             ;      (!) 14.08.05

   freq  dd   11025, 12000, 8000
   Bitrate db 0,1,2, 3, 4, 5, 6, 7, 8,10,12,14,16,18,20,0,\ ; v2 l2 l3
              0,1,2, 3, 4, 5, 6, 7, 8,10,12,14,16,18,20,0,\ ; v2 l2 l3
              0,4,6, 7, 8,10,12,14,16,18,20,22,24,28,32,0,\ ; v2 l1
              0,4,5, 6, 7, 8,10,12,14,16,20,24,28,32,40,0,\ ; v1 l3
              0,4,6, 7, 8,10,12,14,16,20,24,28,32,40,48,0,\ ; v1 l2
              0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,0   ; v1 l1
   xing_offset   db 13, 21, 21, 36
header:
     db   'MP3 Info 0.7',0

S:
    db 'MPEG Version:   .  '
Slen:

L:
    db 'Layer:   '
Llen:

Ka:
    db 'Channels Mode:   '
Kalen:

Su:
    db 'Stereo'
Sulen:

JOS:
    db 'Joint stereo '
JOSlen:

DC:
    db 'Dual channel'
DClen:

SC:
    db 'Single Channel (Mono)'
SClen:

SamR:
    db 'Sample Rate:          Hz'
SamRlen:

BitR:
    db 'BitRate:            Kbps'
BitRlen:

Sizebyte:
    db 'Size:                   bytes'
Sizebytelen:

Timese:
    db 'Time:               seconds'
Timeselen:

frame:
    db 'Frame size:         bytes'
framelen:

framcount:
    db 'Quantity:             frames'
framcountlen:

padding:
    db 'Padding:'
paddinglen:

crci:
    db 'CRC:'
crcilen:

da:
    db 'yes'
dalen:

net:
    db 'no'
netlen:
dots    db      '.  .'
dots_e:

header_found    db      'Header found at:'
header_found_e:

choice:
    db 'File path: '
choicelen:
id3v2   db      'ID3v2'
id3v2_e:
vbr     db      '(VBR)'
vbr_e:
id3v1   db      'ID3v1'
id3v1_e:
track   db      'Track #'
track_e:
title   db      'Title'
title_e:
artist  db      'Artist'
artist_e:
album   db      'Album'
album_e:
year    db      'Year'
year_e:
genre   db      'Genre'
genre_e:
comment db      'Comment'
comment_e:

err_bad_file    db      'Bad file'
err_bad_file_e:

fileinfo2:
  .func  dd 0            ;номер подфункции
  .start dd 0*BLOCK_SIZE ;позиция в файле (в байтах)  *512
         dd 0            ;(зарезервировано под старший dword позиции)
  .size  dd 1*BLOCK_SIZE ;сколько байт читать
  .buf   dd mp3_file     ;указатель на буфер, куда будут записаны данные
  .path:
     db "TEST.MP3",0   ;"/SYS/TEST.MP3",0
     rb 256-($-.path)


;для получения корректного размера файла
fileSizeInfo:
dd 5 ; номер подфункции
dd 0,0,0 ;(зарезервировано)
dd fileBuf ;указатель на буфер, куда будут записаны данные(40 байт)
db 0
dd fileinfo2.path

fileBuf: db 40 dup(0)


;=================================================
   b1s          dd ?   ; standard
   b1l          dd ?   ; layer
   S1           dd ?
   S2           dd ?
   La           dd ?
   shan         dd ?
   K            dd ?
   sam          dd ?
   id           dd ?
   SR           dd ?
   Bita         dd ?
   BR           dd ?
   size         dd ?
   time         dd ?
   frames       dd ?

   xing_bytes   dd ?
   xing_frames  dd ?
   xing_tag     dd ?

   tagv2        dd ?
   tag          dd ?
   .data        rb 128
   framecount   dd ?
   pad          dd ?
   priv         dd ?
   modx         dd ?
   copy         dd ?
   orig         dd ?
   emph         dd ?
   crc          dd ?
   header_at    dd ?
   last_err     dd ?
   last_err_l   dd ?


I_END:

;label  pre_file dword at 0x3000-4
label  mp3_file dword at 0x3000

