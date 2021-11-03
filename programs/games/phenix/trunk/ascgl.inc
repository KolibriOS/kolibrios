lang equ ru

;
;   Assembler
;     SMALL
;       CODE
;         Graphics
;           Libary
;
;   Ver 0.18
;

; draw image into image with alpha color mask
; image_draw_acimage dest, source, x, y, alpha_color
;
macro image_draw_acimage dest, source, x, y, alpha_color
{
local next_pix,next_line,skip_pix,no_skip
   push dest
   push source
   push x
   push y
   push alpha_color
   pop ebp  ; alpha color
   pop eax  ; y
   pop ebx  ; x
   pop esi  ; src
   pop edi  ; dest
   call image_draw_acimage_proc

if ~ defined image_draw_acimage_used
image_draw_acimage_used equ 1

   jmp end_image_draw_acimage_proc
image_draw_acimage_proc:
   mov ecx,dword [edi]   ; ecx = canvas width
   mul ecx       ; edx:eax = ypos * canvas width
   add eax,ebx   ; eax =  (ypos * canvas width) + xpos
   lea eax,[eax+eax*2+8] ; eax=(y*xsize+x)*3+8 (8=skip xy size dwords)

   mov edx,[esi]   ; edx = img width
   sub ecx,edx     ; ecx = canvas width - img width
   lea ebx,[ecx*2+ecx] ; ebx = how many pixels skip for new line
   mov ecx,[esi+4] ; ecx = img height
   add esi,8       ; esi + 8 for skip xy size dwords
   add edi,eax     ; edi = dest position
   shl ebp,8       ; for fast compare with alpha color
   cld             ; set movs direction flag
next_line:
   push edx
next_pix:
   mov eax,[esi]
   shl eax,8
   cmp eax,ebp
   je  skip_pix
   movsw
   movsb
   jmp no_skip
skip_pix:
   add esi,3
   add edi,3
no_skip:
   dec edx
   jnz next_pix
   pop edx
   add edi,ebx
   dec ecx
   jnz next_line
   ret
end_image_draw_acimage_proc :
end if

}

macro image_set_size image,width,height
{
   mov dword [image],width
   mov dword [image+4],height
}

; label - draw label on window
; example:
; label 10,12,'Hello World!',cl_Green+font_size_x4+utf16

macro image_draw_label image,x,y,text,color
{
local label_text
   draw_to_buffer   equ 00001000b shl 24
   jmp @f
label_text db text
@@:
   words2reg ebx,x,y       ; ebx - position
   dword2reg ecx,color+draw_to_buffer ; ecx - color
   mov edi,image
   mov edx,label_text      ; edx - address of label text
   mov esi,@b-label_text   ; esi - size of libel in bytes
   mov eax,4
   mcall
}

; draw_frect - draw filled rect
macro draw_frect x,y,xs,ys,color
{
   wordstoreg ebx,x,xs ;x*65536+xs
   wordstoreg ecx,y,ys ;y*65536+ys
   mov  edx,color
   mov  eax,13
   mcall
}

; draw_label - Draw label in window
; example:
; draw_label 10,12,'Hello World!',cl_Green+font_size_x4+utf16

macro draw_label x,y,text,color
{
local label_text
   words2reg ebx,x,y   ; ebx - position
if text eqtype 123 | text eqtype eax
   movt edx,text
   mov ecx,color+(1 shl 31)  ; ecx - color
else
   mov edx,label_text  ; edx - address of label text
   jmp @f
label_text db text
@@:
   mov esi,@b-label_text     ; esi - size of libel in bytes
   movt ecx,color            ; ecx - color
end if
   mov eax,4
   mcall
}

hide_zeros equ (1 shl 31)
use_bg_color equ (1 shl 30)
use_big_font equ (1 shl 28)

macro draw_number data, x, y, color, numtype, bg_color
{
   movt ecx,data
   movt ebx,numtype
   mov bl,0     ; if bl = 0, ecx is contain number
   words2reg edx,x,y
if bg_color eq
   movt esi,color
else
   movt esi,color+use_bg_color
   movt edi,bg_color
end if
   mov eax,47
   mcall
}

; draw_image - macro for draw image on window area
macro draw_image x, y, image
{
   mov ecx,[image-2]    ; -2 for except shl ecx,16
   mov cx,[image+4]     ; ecx = xsize*65536+ysize
   wordstoreg edx, x, y ; edx = x*65536+y
   lea ebx,[image+8]    ; ebx = image data address
   mov eax,7            ; eax = 7 is draw image function
   mcall
}

aframetoimg_use_count=0
macro aframetoimg img, x, y, canvas,acol
{
local loo,loo2,acolor
aframetoimg_use_count=aframetoimg_use_count+1
if aframetoimg_use_count = 1

   jmp end_aframetoimg_proc

acolor dd 0
aframetoimg_proc:
;getout coord
   mov [acolor],ebp

   mov edx,ebx ;img   ;xsize
   movzx eax,word [edx]
   add eax,esi ;y cor

;    mov eax,esi ;y cor
   mul dword [ecx] ;canvas xsize
   add eax,edi ;x cor

   mov ebp,ebx ;img   ;xsize
   movzx edx,word [ebp]
   add eax,edx

   mov ebp,eax
   shl eax,1
   add ebp,eax
   add ebp,ecx ;canvas+8;start
   add ebp,8
;get img size
   add ebx,4
   mov eax,ebx ;img   ;xsize
   movzx esi,word [eax]
   movzx edi,word [eax+2]
   add ebx,4
   mov edx,ebx ;img+8
loo2:
push esi
loo:
;test on alpha color
   mov eax,[edx]
   shl eax,8
   shr eax,8
   cmp eax,[acolor]
   jne  yx
   add edx,3
   add ebp,3
   jmp nx
yx:
   mov al,byte [edx]
   mov byte [ebp],al
   inc ebp
   inc edx
   mov al,byte [edx]
   mov byte [ebp],al
   inc ebp
   inc edx
   mov al,byte [edx]
   mov byte [ebp],al
   inc ebp
   inc edx
nx:
   dec esi
   jnz loo
pop esi
   sub ebp,3
   mov eax,[ecx]  ;offset = offset+((canxsize-imgxsize)*3)
   sub eax,esi
   add ebp,eax
   shl eax,1
   add ebp,eax

   add ebp,3

   dec edi
   jnz loo2
   ret
end_aframetoimg_proc:
end if
   push img
   push canvas
   push x
   push y
   push acol
   pop  ebp
   pop  esi
   pop  edi
   pop ecx
   pop ebx
   call aframetoimg_proc
}

frametoimg_use_count=0
macro frametoimg img, x, y, canvas
{
local loo,loo2
frametoimg_use_count=frametoimg_use_count+1
if frametoimg_use_count = 1

   jmp end_frametoimg_proc

frametoimg_proc:
;getout coord
   mov edx,ebx ;img   ;xsize
   movzx eax,word [edx]
   add eax,esi ;y cor

;    mov eax,esi ;y cor
   mul dword [ecx] ;canvas xsize
   add eax,edi ;x cor

   mov ebp,ebx ;img   ;xsize
   movzx edx,word [ebp]
   add eax,edx

   mov ebp,eax
   shl eax,1
   add ebp,eax
   add ebp,ecx ;canvas+8;start
   add ebp,8
;get img size
   add ebx,4
   mov eax,ebx ;img   ;xsize
   movzx esi,word [eax]
   movzx edi,word [eax+2]
   add ebx,4
   mov edx,ebx ;img+8
loo2:
push esi
loo:
   mov al,byte [edx]
   mov byte [ebp],al
   inc ebp
   inc edx
   mov al,byte [edx]
   mov byte [ebp],al
   inc ebp
   inc edx
   mov al,byte [edx]
   mov byte [ebp],al
   inc ebp
   inc edx

   dec esi
   jnz loo
pop esi
   sub ebp,3
   mov eax,[ecx]  ;offset = offset+((canxsize-imgxsize)*3)
   sub eax,esi
   add ebp,eax
   shl eax,1
   add ebp,eax

   add ebp,3

   dec edi
   jnz loo2
   ret
end_frametoimg_proc:
end if
   push img
   push canvas
   push x
   push y
   pop  esi
   pop  edi
   pop ecx
   pop ebx
   call frametoimg_proc
}


imgtoimg_use_count=0
macro imgtoimg img, x, y, canvas
{
local loo,loo2
imgtoimg_use_count=imgtoimg_use_count+1
if imgtoimg_use_count = 1

   jmp end_imgtoimg_proc
imgtoimg_proc:
;getout coord
   mov eax,esi ;y cor
   mul dword [ecx] ;canvas xsize
   add eax,edi ;x cor
   mov ebp,eax
   shl eax,1
   add ebp,eax
   add ebp,ecx ;canvas+8;start
   add ebp,8
;get img size
   mov eax,ebx ;img   ;xsize
   mov esi,[eax]
   add ebx,4
   mov eax,ebx ; img+4 ;ysize
   mov edi,[eax]
   add ebx,4
   mov edx,ebx ;img+8
loo2:
push esi
loo:
   mov al,byte [edx]
   mov byte [ebp],al
   inc ebp
   inc edx
   mov al,byte [edx]
   mov byte [ebp],al
   inc ebp
   inc edx
   mov al,byte [edx]
   mov byte [ebp],al
   inc ebp
   inc edx
   dec esi
   jnz loo
pop esi
   sub ebp,3
   mov eax,[ecx]  ;offset = offset+((canxsize-imgxsize)*3)
   sub eax,esi
   add ebp,eax
   shl eax,1
   add ebp,eax
   add ebp,3
   dec edi
   jnz loo2
   ret
end_imgtoimg_proc:
end if
   push img
   push canvas
   push x
   push y
   pop  esi
   pop  edi
   pop  ecx
   pop  ebx
   call imgtoimg_proc
}

; FPS - Set Frame Per Second Display
fps_show_frequency=40
macro fps x,y,color,delcolor
{
local spdat,savetime,new_time,fps,fps_cntr,out_fps,new_time,ttt
local no_out_fps
   jmp spdat
savetime dd 0
fps_cntr dd 0
fps      dd 0
ttt      dd 0
spdat:
get_time:
   mov eax,3
   mcall
   cmp eax,[savetime]
   jne new_time
   inc [fps_cntr]
   cmp dword [ttt],0
   je  out_fps
   dec dword [ttt]
   jmp no_out_fps
new_time:
   mov [savetime],eax
   mov ebx,[fps_cntr]
   mov [fps],ebx
   mov [fps_cntr],0
out_fps:
if ~(delcolor eq )
   mov ebx,x*65536+30
   mov ecx,y*65536+7
   mov edx,delcolor
   mov eax,13
   mcall
end if
   mov dword [ttt],fps_show_frequency
   mov eax,47
   mov ebx,5*65536
   ;   mov bl,0
   mov edx,x*65536+y
   mov esi,color
   mov ecx,[fps]
   mcall
no_out_fps:
}

macro rgbtobgr image
{
   mov eax,[image]
   mul dword [image+4]
   mov ecx,eax
   mov esi,image+8
@@:
   mov al,[esi]
   mov bl,[esi+2]
   mov [esi],bl
   mov [esi+2],al
   add esi,3
   dec ecx
   jnz @b
}

macro setframe x , y ,arg3
{
   mov  eax,7
   mov  ebx,arg3
   add  ebx,8
   wordstoreg edx, x , y  ;arg1*65536+arg2
   add  edx,dword [arg3]
   mov  ecx,dword [arg3+4]
   mcall
}


macro getimg imgsrc,x,y,xs,ys,imgdest
{
local next_pixel,next_line
; store image size
if xs eqtype 0 | xs eqtype eax
   mov dword [imgdest],xs
else
   mov eax,xs
   mov dword [imgdest],eax
end if
if ys eqtype 0 | ys eqtype eax
   mov dword [imgdest+4],ys
else
   ;push ys
   ;pop dword [imgdest+4]
   mov eax,ys
   mov dword [imgdest+4],eax
end if

   lea edi,[8+imgdest]    ; edi = destinaton address
   mov eax,dword [imgsrc] ; eax = xsize of source image in pixels
   push eax               ; store eax before mul operation
   mov edx,y
   mul edx                ; edx:eax = eax*edx
   add eax,x
   lea esi,[imgsrc+8+eax+2*eax]    ; esi = start offset on img src
   pop eax                ; restore eax
   sub eax,xs             ; eax = src image xsize - crop fragment xsize
   lea eax,[eax+eax*2]    ; eax = eax * 3 (bytes per pixel)

   ; this loop used esi,edi,ecx,edx,eax registers
   mov edx,ys     ; edx = ysize in pixels
   cld            ; set direction
next_line:
   mov ecx,xs     ; ecx = xsize in pixels
next_pixel:
   movsw
   movsb          ; write 3 bytes pixel
   dec ecx
   jnz next_pixel
   add esi,eax
   dec edx
   jnz next_line
}

macro copyimg img2_off,img1_off
{
   mov  eax,dword [img1_off]
   mov  ebx,dword [img1_off+4]
   mul  ebx
   lea  ecx,[eax+2*eax]
   lea  esi,[img1_off+8]
   lea  edi,[img2_off+8]
   cld
   rep  movsb
}

macro fullimg img_off,xs,ys,color
{
local cop
   mov eax,xs
   mov ebx,ys
   mov  dword [img_off],eax
   mov  dword [img_off+4],ebx
   mul  ebx
   lea  ebp,[eax+2*eax]
   mov  esi,color
if color eqtype 0
   mov  ecx,color/65536
else
   mov  ecx,esi
   shr  ecx,16
end if
   xor  edi,edi
cop:
   mov  word [img_off+8+edi],si
   add  edi,2
   mov  byte [img_off+8+edi],cl
   inc  edi
   cmp  edi,ebp
   jne  cop
}

; number of frame in ecx
; callculatin offset of raw data

macro getframeoff num_of_frame,offset_of_animation,offset_of_frame
{
local loo,setpic
  mov ebp,num_of_frame ;ecx
  mov esi,offset_of_animation;Image
loo:
  cmp ebp,0
  je  setpic
  movzx eax,word [esi+4]
  movzx ebx,word [esi+6]
  mul ebx ;dword [esi+4]
  mov ebx,3
  mul ebx
  add eax,8
  add esi,eax
  dec ebp
  jmp loo
setpic:
  mov dword offset_of_frame,esi
}

; BMPTOIMG -Convert BMP format TO IMG format
; (SYNTAX)  BMPTOIMG BMP_source_offset,IMG_dest_ofset
; (SAMPLE)  View BMPLS.ASM sample.
; ( NOTE )  This is macro is not brake bmp structure! Tested in 32,8,4 bits

bmptoimg_data_area_count=0
macro bmptoimg bmp_load_area,img_dest_area
{
local fileinfo,string,end_bmp,nodix
local converttable,noaddelem,nextbit,convert1bpp,convert4bpp,convert2
local nextelem,convertno32,nomorestring,convert1,nextstring,yespicsize
;local qwe,bmpfn

;  convert:
   movzx eax,word [bmp_load_area+28]
   mul  dword [bmp_load_area+18]
   add  eax,31
   shr  eax,5
   mov  dword [bmptoimg_data_area_dwps],eax  ;dwps-doublewords per string
   shl  eax,2
   mov  dword [bmptoimg_data_area_bps],eax   ;bps-bytes per string

   cmp dword [bmp_load_area+34],0
   jne  yespicsize  ;if picture size is defined
   mul dword [bmp_load_area+22]
   mov dword [bmp_load_area+34],eax

yespicsize:
   mov ebp,img_dest_area+8

   mov  eax,bmp_load_area
   mov  ebx,eax
   add  ebx, [bmp_load_area+2];file size
   inc  ebx
   mov  dword [bmptoimg_soi],ebx   ;soi-start of image area for drawing

   add  eax, [bmp_load_area+10]
   mov  dword [bmptoimg_data_area_sop],eax   ;sop-start of picture in file
   add  eax, [bmp_load_area+34]
   mov  dword [bmptoimg_data_area_eop],eax   ;eop-end of picture in file
   mov  eax, [bmp_load_area+18]
   lea  eax,[eax+2*eax]   ;3x pixels in eax

   mov  edi,dword [bmptoimg_soi]   ;initializing
   mov  esi,dword [bmptoimg_data_area_eop]
   sub  esi,dword [bmptoimg_data_area_bps]


nextstring:
   push edi
   push ebp
   cmp  word [bmp_load_area+28],24
   jne  convertno32

   mov edi,ebp
   mov  ecx,[bmptoimg_data_area_dwps]
   cld
   rep movsd

convert1:
   pop  ebp
   pop  edi
   sub  esi,dword [bmptoimg_data_area_bps]
   sub  esi,dword [bmptoimg_data_area_bps]
   cmp  esi,dword [bmptoimg_data_area_sop]
   jb   end_bmp
   add  edi,eax
   add  ebp,eax
   jmp  nextstring

convertno32:
   mov  ebx,bmp_load_area
   add  ebx, [bmp_load_area+14]
   add  ebx,14          ;start of color table
   push esi
   add  esi,dword [bmptoimg_data_area_bps]
   mov  dword [bmptoimg_data_area_eos],esi
   pop  esi
nextelem:
   push eax
   movzx eax,byte [esi]
   cmp  word [bmp_load_area+28],4
   je   convert4bpp
   cmp  word [bmp_load_area+28],1
   je   convert1bpp
   call converttable

convert2:
   pop  eax
   inc  esi
   cmp  esi,dword [bmptoimg_data_area_eos]
   jae  convert1
   add  edi,3
   add  ebp,3
   jmp  nextelem

convert4bpp:
   shl  ax,4
   shr  al,4
   push ax
   movzx eax,ah
   call converttable
   add  edi,3
   add ebp,3
   pop  ax
   movzx eax,al
   call converttable
   jmp  convert2

convert1bpp:
   mov  ecx,eax
   mov  edx,7
nextbit:
   xor  eax,eax
   bt   ecx,edx
   jnc  noaddelem
   inc  eax
noaddelem:
   push edx
   call converttable
   pop  edx
   dec  edx
   js   convert2
   add  edi,3
   add  ebp,3
   jmp  nextbit
converttable:
   shl  eax,2
   add  eax,ebx
   mov  edx, dword [eax]
   ;    mov  dword [edi],edx
   mov [ebp],edx
   ret

bmptoimg_data_area_count=bmptoimg_data_area_count+1
if bmptoimg_data_area_count = 1
; DATA AREA
bmptoimg_soi                dd 0
bmptoimg_data_area_bps      dd 0
bmptoimg_data_area_dwps     dd 0
bmptoimg_data_area_sop      dd 0
bmptoimg_data_area_eop      dd 0
bmptoimg_data_area_eos      dd 0
end if

end_bmp:
   mov  eax,dword [bmp_load_area+18]
   mov  ebx,dword [bmp_load_area+22]
   mov  dword [img_dest_area],eax
   mov  dword [img_dest_area+4],ebx
}

if used ReadGIF
; For convert RGB to BGR
COLOR_ORDER equ MENUETOS
include 'gif_lite.inc'
end if

macro giftoani gifsrc,imgsrc,num_of_frames
{
gif_img_count = num_of_frames
   mov     esi, gifsrc
   mov     edi, imgsrc
   call    ReadGIF
}

macro giftoimg gifsrc,imgsrc
{
if defined gif_img_count
error 'giftoimg cannot be used in GIF multiple images mode. Use giftoani instead.'
end if
   mov     esi, gifsrc
   mov     edi, imgsrc
   call    ReadGIF
}
