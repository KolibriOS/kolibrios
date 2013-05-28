  encoding_raw:
    DEBUGF 1,'RAW\n'

    mov   ax,[frame.y]			      ;
    mov   bx,[screen.width]		      ;
    mul   bx				      ;
    shl   edx,16			      ;
    mov   dx,ax 			      ; [screen.width]*[frame.y]
    movzx eax,[frame.x]
    add   edx,eax			      ; [screen.width]*[frame.y]+[frame.x]

    mov   eax,3 			      ;
    mul   edx				      ; ([screen.width]*[frame.y]+[frame.x])*3

    add   eax,framebuffer_data		      ;
    push  eax				      ; framebuffer_data+([screen.width]*[frame.y]+[frame.x])*3

    mov   ax,[frame.width]		      ;
    mov   bx,3				      ;
    mul   bx				      ;
    shl   edx,16			      ;
    mov   dx,ax 			      ; [frame.width]*3

    pop   eax				      ;
    add   edx,eax			      ; framebuffer_data+([screen.width]*[frame.y]+[frame.x])*3+[frame.width]*3
    push  eax				      ;
    push  edx				      ;

    mov   ax,[frame.height]		      ;
    dec   ax				      ;
    mov   bx,3				      ;
    mul   bx				      ;
    mov   bx,[screen.width]		      ;
    mul   bx				      ;
    shl   edx,16			      ;
    mov   dx,ax 			      ;
    mov   ecx,edx			      ;
    pop   edx				      ;
    add   ecx,edx			      ; mov ecx,edx+([frame.height]-1)*[screen.width]*3
    pop   ebx

   .pixelloop32:
    cmp   ebx,ecx
    jge   next_rectangle

;    add   esi,2           ; 32 bit code RAW - OK
;    mov   al,[esi]        ;
;    mov   [ebx],al        ;
;    inc   ebx             ;
;    dec   esi             ;
;                          ;
;    mov   al,[esi]        ;
;    mov   [ebx],al        ;
;    inc   ebx             ;
;    dec   esi             ;
;                          ;
;    mov   al,[esi]        ;
;    mov   [ebx],al        ;
;    inc   ebx             ;
;    add   esi,4           ;

;    push  ecx             ; 16 bit code RAW
;    mov   cl,51
;
;    mov   ax,[esi]        ;
;        xchg  al,ah
;    and   al,00011111b    ;
;        xchg  al,ah
;    mul   cl
;    mov   [ebx],al        ;
;    inc   ebx             ;
;
;    mov   ax,[esi]        ;
;        xchg  al,ah
;    shr   ax,5            ;
;        xchg  al,ah
;    and   al,00011111b    ;
;    mul   cl
;    mov   [ebx],al        ;
;    inc   ebx             ;
;
;    mov   ax,[esi]        ;
;    xchg  al,ah
;    shr   ax,10           ;
;    and   al,00011111b    ;
;    mul   cl
;    mov   [ebx],al        ;
;    inc   ebx             ;
;
;    inc   esi             ;
;    inc   esi             ;
;    pop   ecx

    push  ecx		  ; 8 bit code RAW - OK
    mov   cl,85 	  ;
			  ;
    mov   al,[esi]	  ;
    shr   al,4		  ;
    and   al,3		  ;
    mul   cl		  ;
    mov   [ebx],al	  ;
    inc   ebx		  ;
			  ;
    mov   al,[esi]	  ;
    shr   al,2		  ;
    and   al,3		  ;
    mul   cl		  ;
    mov   [ebx],al	  ;
    inc   ebx		  ;
			  ;
    mov   al,[esi]	  ;
    and   al,3		  ;
    mul   cl		  ;
    mov   byte[ebx],al	  ;
    inc   ebx		  ;
    inc   esi		  ;
    pop   ecx		  ;


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



