  encoding_raw:
    DEBUGF 1,'FRAME: RAW\n'

    mov   ax,[screen.width]		      ;
    mov   bx,[frame.y]			      ;
    mul   bx				      ;
    DEBUGF 1,'screen.width*frame.y=%u\n',ax
    mov   bx,3				      ;
    mul   bx				      ;
    shl   edx,16			      ;
    mov   dx,ax 			      ;
    mov   ebx,edx			      ; mov ebx,[screen.width]*[frame.y]*3
    push  ebx
    DEBUGF 1,'screen.width*frame.y*3=%u\n',edx
    mov   ax,[frame.x]			      ;
    mov   bx,3				      ;
    mul   bx				      ;
    shl   edx,16			      ;
    mov   dx,ax 			      ;
    pop   ebx
    add   ebx,edx			      ; add ebx,[frame.x]*3
    add   ebx,framebuffer_data		      ; add ebx,framebuffer_data
    push  ebx
    DEBUGF 1,'frame.x*3=%u\n',edx

    mov   ax,[frame.width]		      ;
    mov   bx,3				      ;
    mul   bx				      ;
    shl   edx,16			      ;
    mov   dx,ax 			      ;
    DEBUGF 1,'frame.width*3=%u\n',edx
    pop   ebx
    add   edx,ebx			      ; mov edx,ebx+[frame.width]*3

    push  ebx
    push  edx				      ;
    mov   ax,[frame.height]		      ;
    mov   bx,3				      ;
    mul   bx				      ;
    mov   bx,[screen.width]		      ;
    mul   bx				      ;
    shl   edx,16			      ;
    mov   dx,ax 			      ;
    mov   ecx,edx			      ;
    pop   edx				      ;
    DEBUGF 1,'frame.height*screen.width*3=%u\n',ecx
    add   ecx,edx			      ; mov ecx,edx+[frame.height]*[screen.width]*3
    pop   ebx

    DEBUGF 1,'FRAME: framebuffer:%u ebx:%u ecx:%u edx:%u\n',framebuffer_data,ebx,ecx,edx

   .pixelloop32:
    cmp   ebx,ecx
    jge   next_rectangle

    add   esi,2 	  ; 32 bit code RAW
    mov   al,[esi]	  ;
    mov   [ebx],al	  ;
    inc   ebx		  ;
    dec   esi		  ;
			  ;
    mov   al,[esi]	  ;
    mov   [ebx],al	  ;
    inc   ebx		  ;
    dec   esi		  ;
			  ;
    mov   al,[esi]	  ;
    mov   [ebx],al	  ;
    inc   ebx		  ;
    add   esi,4 	  ;

;    mov   ax,[esi]        ; 16 bit code RAW
;    and   ax,32
;    mov   [ebx],al
;    inc   ebx
;
;    mov   ax,[esi]
;    shr   ax,5
;    and   ax,32
;    mov   [ebx],al
;    inc   ebx
;
;    mov   ax,[esi]
;    shr   ax,10
;    and   ax,64
;    mov   [ebx],al
;    inc   ebx
;    inc   esi
;    inc   esi


;    mov   al,[esi]        ; 8 bit code RAW
;    and   al,7            ;
;    mov   byte[ebx],0xff;al        ;
;    inc   ebx             ;
;                          ;
;    mov   al,[esi]        ;
;    shr   al,3            ;
;    and   al,7            ;
;    mov   [ebx],al        ;
;    inc   ebx             ;
;                          ;
;    mov   al,[esi]        ;
;    shr   al,6            ;
;    and   al,3            ;
;    mov   [ebx],al        ;
;    inc   ebx             ;
;                          ;
;    inc   esi             ;


    cmp   ebx,edx
    jl	  .pixelloop32

    push  edx
    push  ebx
    mov   ax,[screen.width]
    mov   bx,3
    mul   bx
    shl   edx,16
    mov   dx,ax
    mov   eax,edx
    pop   ebx
    pop   edx

    add   ebx,eax ; eax = [screen.width]*3
    add   edx,eax

    push  edx
    push  ebx
    mov   ax,[frame.width]
    mov   bx,3
    mul   bx
    shl   edx,16
    mov   dx,ax
    mov   eax,edx
    pop   ebx
    pop   edx

    sub   ebx,eax ; eax = [frame.width]*3

    jmp   .pixelloop32



