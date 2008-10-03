;-----------------------------------------------------------------------------
;///// PART OF ATi RADEON 9000 DRIVER ////////////////////////////////////////
;-----------------------------------------------------------------------------
; Copyright (c) 2004, mike.dld
; Using BeOS driver - Copyright (c) 2002, Thomas Kurschel
;-----------------------------------------------------------------------------
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
; DEALINGS IN THE SOFTWARE.
;-----------------------------------------------------------------------------

include 'clipping.inc'

struct RECT
 left	dd ?
 top	dd ?
 right	dd ?
 bottom dd ?
ends

virtual at ebp
 r RECT
end virtual

virtual at edi
 r2 RECT
end virtual

SR = sizeof.RECT

macro movl val,reg {
 mov reg,val
}

EQUAL_LEFT   = 00000001b
EQUAL_BOTTOM = 00000010b
EQUAL_RIGHT  = 00000100b
EQUAL_TOP    = 00001000b

func calc_clipping_rects
begin
	mov	[cnt],0
	movzx	ebp,word[CURRENT_TASK]
	shl	ebp,5

	cmp	ebp,0x20
	jne	@f
	mov	esi,viewport
	mov	edi,rct
	mov	ecx,sizeof.RECT/4
	cld
	rep	movsd
	jmp	.lp1
    @@:
	movsx	eax,word[ebp+0x00]
	mov	[rct.left],eax
	mov	[rct.right],eax
	movsx	eax,word[ebp+0x04]
	mov	[rct.top],eax
	mov	[rct.bottom],eax
	movzx	eax,word[ebp+0x08]
	inc	eax
	add	[rct.right],eax
	movzx	eax,word[ebp+0x0C]
	inc	eax
	add	[rct.bottom],eax
  .lp1:
	mov	esi,viewport
	mov	edi,tr
	mov	ecx,sizeof.RECT/4
	cld
	rep	movsd
	mov	ebp,rct
	call	intersect_rect ; (ebp,tr)->(x1:edx,y1:eax,x2:ebx,y2:ecx)+CF
	jc	.exit
	mov	[rct.left],edx
	mov	[rct.top],eax
	mov	[rct.right],ebx
	mov	[rct.bottom],ecx
	inc	[cnt]
comment ^
	movsx	eax,word[ebp+0x00]
	mov	[rct.left],eax
	mov	[rct.right],eax
	movsx	eax,word[ebp+0x04]
	mov	[rct.top],eax
	mov	[rct.bottom],eax
	movsx	eax,word[ebp+0x08]
	inc	eax
	add	[rct.right],eax
	movsx	eax,word[ebp+0x0C]
	inc	eax
	add	[rct.bottom],eax
^
	movzx	ecx,word[TASK_COUNT]	; number of processes
	jif	ecx,be,1,.exit

; calculate clipping rectangles

	mov	esi,1
  ; go forward through all windows
    .next_window:
	movzx	edi,word[CURRENT_TASK]	; calling process number

	mov	ax,[WIN_STACK+esi*2]
	jif	ax,be,[WIN_STACK+edi*2],.end_window.2

	mov	ebp,[cnt]
	shl	ebp,4			; ebp *= SR
	jz	.exit
	lea	ebp,[rct+ebp-SR]

	push	esi ;ecx esi
	shl	esi,5
	lodsd
	mov	[tr.left],eax
	mov	[tr.right],eax
	lodsd
	mov	[tr.top],eax
	mov	[tr.bottom],eax
	lodsd
	jif	eax,z,eax,.end_window,test
	inc	eax
	add	[tr.right],eax
	lodsd
	jif	eax,z,eax,.end_window,test
	inc	eax
	add	[tr.bottom],eax
  ; go backward through all rectangles
    .next_rect:

rc_top	  equ eax
rc_right  equ ebx
rc_bottom equ ecx
rc_left   equ edx

	call	intersect_rect ; (ebp,tr)->(x1:edx,y1:eax,x2:ebx,y2:ecx)+CF
	jc	.is_finish
	xor	edi,edi
	jif	rc_top,ne,[r.top],@f
	or	edi,EQUAL_TOP
    @@: jif	rc_right,ne,[r.right],@f
	or	edi,EQUAL_RIGHT
    @@: jif	rc_bottom,ne,[r.bottom],@f
	or	edi,EQUAL_BOTTOM
    @@: jif	rc_left,ne,[r.left],@f
	or	edi,EQUAL_LEFT
    @@: jmp	[jtable_intersect+edi*4]

    .is_0000:
	call	copy_current
	mov	[r.left      ],rc_right
	mov	[r2.right -SR],rc_left
	mov	[r2.left     ],rc_left
	mov	[r2.right    ],rc_right
	mov	[r2.bottom   ],rc_top
	mov	[r2.left  +SR],rc_left
	mov	[r2.right +SR],rc_right
	mov	[r2.top   +SR],rc_bottom
	movl	[r.top	     ],esi
	mov	[r2.top      ],esi
	movl	[r.bottom    ],esi
	mov	[r2.bottom+SR],esi
	add	[cnt],2
	jmp	.is_finish
    .is_0001:
	call	copy_current
	mov	[r.top	     ],rc_bottom
	mov	[r2.bottom-SR],rc_top
	mov	[r2.left     ],rc_right
	mov	[r2.top      ],rc_top
	mov	[r2.bottom   ],rc_bottom
	movl	[r.right     ],esi
	mov	[r2.right    ],esi
	inc	[cnt]
	jmp	.is_finish
    .is_0010:
	call	copy_current
	mov	[r.left      ],rc_right
	mov	[r2.right -SR],rc_left
	mov	[r2.left     ],rc_left
	mov	[r2.right    ],rc_right
	mov	[r2.bottom   ],rc_top
	movl	[r.top	     ],esi
	mov	[r2.top      ],esi
	inc	[cnt]
	jmp	.is_finish
    .is_0011:
	call	copy_current
	mov	[r.bottom    ],rc_top
	mov	[r2.left  -SR],rc_right
	mov	[r2.top   -SR],rc_top
	jmp	.is_finish
    .is_0100:
	call	copy_current
	mov	[r.top	     ],rc_bottom
	mov	[r2.bottom-SR],rc_top
	mov	[r2.right    ],rc_left
	mov	[r2.top      ],rc_top
	mov	[r2.bottom   ],rc_bottom
	movl	[r.left      ],esi
	mov	[r2.left     ],esi
	inc	[cnt]
	jmp	.is_finish
    .is_0101:
	call	copy_current
	mov	[r.top	     ],rc_bottom
	mov	[r2.bottom-SR],rc_top
	jmp	.is_finish
    .is_0110:
	call	copy_current
	mov	[r.bottom    ],rc_top
	mov	[r2.right -SR],rc_left
	mov	[r2.top   -SR],rc_top
	jmp	.is_finish
    .is_0111:
	mov	[r.bottom    ],rc_top
	jmp	.is_finish
    .is_1000:
	call	copy_current
	mov	[r.left      ],rc_right
	mov	[r2.right -SR],rc_left
	mov	[r2.left     ],rc_left
	mov	[r2.right    ],rc_right
	mov	[r2.top      ],rc_bottom
	movl	[r.bottom    ],esi
	mov	[r2.bottom   ],esi
	inc	[cnt]
	jmp	.is_finish
    .is_1001:
	call	copy_current
	mov	[r.top	     ],rc_bottom
	mov	[r2.left  -SR],rc_right
	mov	[r2.bottom-SR],rc_bottom
	jmp	.is_finish
    .is_1010:
	call	copy_current
	mov	[r.left      ],rc_right
	mov	[r2.right -SR],rc_left
	jmp	.is_finish
    .is_1011:
	mov	[r.left      ],rc_right
	jmp	.is_finish
    .is_1100:
	call	copy_current
	mov	[r.top	     ],rc_bottom
	mov	[r2.right -SR],rc_left
	mov	[r2.bottom-SR],rc_bottom
	jmp	.is_finish
    .is_1101:
	mov	[r.top	     ],rc_bottom
	jmp	.is_finish
    .is_1110:
	mov	[r.right     ],rc_left
	jmp	.is_finish
    .is_1111:
	call	delete_current
    .is_finish:
	sub	ebp,SR
	jif	ebp,ae,rct,.next_rect
    .end_window:
	pop	esi; ecx
    .end_window.2:
	inc	esi
	jif	esi,be,[TASK_COUNT],.next_window
;       dec     ecx
;       jnz     .next_window

; combine some rectangles if possible
; with Result do begin
;  for i := cnt-1 downto 0 do if rct[i].Left >= 0 then
;   for j := cnt-1 downto 0 do if (j <> i) and (rct[j].Left >= 0) then
;    if (rct[i].Left = rct[j].Left) and (rct[i].Right = rct[j].Right) then begin
;    end else if (rct[i].Top = rct[j].Top) and (rct[i].Bottom = rct[j].Bottom) then begin
;     if (rct[i].Left = rct[j].Right) then begin
;      rct[i].Left := rct[j].Left;
;      rct[j].Left := -1;
;     end else if (rct[i].Right = rct[j].Left) then begin
;      rct[i].Right := rct[j].Right;
;      rct[j].Left := -1;
;     end;
;    end;
;  for i := cnt-1 downto 0 do if rct[i].Left < 0 then begin
;   for j := i to cnt-2 do
;    rct[j] := rct[j+1];
;   dec(cnt);
;  end;
; end;

.combine_rects:
       mov     esi,[cnt]
       shl     esi,4
       add     esi,rct
       lea     ebp,[esi-SR]
       push    ebp
  .next_rect1:
       sub     esi,SR
       jif     esi,b,rct,.exit.combine
       jif     [esi+RECT.left],e,-1,.next_rect1
       push    ebp
  .next_rect2:
       jif     ebp,e,esi,.next_rect2.ok
       jif     [ebp+RECT.left],e,-1,.next_rect2.ok

       mov     eax,[ebp+RECT.left]
       mov     ebx,[ebp+RECT.right]
       mov     ecx,[ebp+RECT.top]
       mov     edx,[ebp+RECT.bottom]
       jif     eax,ne,[esi+RECT.left],.not_left_right
       jif     ebx,ne,[esi+RECT.right],.not_left_right
       jif     edx,ne,[esi+RECT.top],@f
       mov     [esi+RECT.top],ecx
       jmp     .next_rect2.mark
    @@:        jif     ecx,ne,[esi+RECT.bottom],.next_rect2.ok
       mov     [esi+RECT.bottom],edx
       jmp     .next_rect2.mark
  .not_left_right:
       jif     ecx,ne,[esi+RECT.top],.next_rect2.ok
       jif     edx,ne,[esi+RECT.bottom],.next_rect2.ok
       jif     ebx,ne,[esi+RECT.left],@f
       mov     [esi+RECT.left],eax
       jmp     .next_rect2.mark
    @@:        jif     eax,ne,[esi+RECT.right],.next_rect2.ok
       mov     [esi+RECT.right],ebx
  .next_rect2.mark:
       or      [ebp+RECT.left],-1
  .next_rect2.ok:
       sub     ebp,SR
       jif     ebp,ae,rct,.next_rect2
       pop     ebp
       jmp     .next_rect1
  .exit.combine:

       pop     ebp
  .next_rect3:
       jif     [ebp+RECT.left],ne,-1,@f
       call    delete_current
    @@:        sub     ebp,SR
       jif     ebp,ae,rct,.next_rect3

; remove unnecessary rectangles
;  for i := Result.cnt-1 downto 0 do with Result do
;   if not IntersectRect(rc,rct[i],r2) then begin
;    for j := i to cnt-2 do
;     rct[j] := rct[j+1];
;    dec(cnt);
;   end;

    .exit:
	mov	esi,rct
	mov	ecx,[cnt]
	ret

delete_current:
	push	ecx
	lea	esi,[ebp+SR]	; esi = ebp+SR
	mov	edi,ebp 	; edi = ebp
	mov	ecx,[cnt]	; ecx = cnt
	shl	ecx,4		; ecx *= SR
	add	ecx,rct-SR	; ecx += rct-SR
	sub	ecx,ebp 	; ecx -= ebp
	cld
	rep	movsb
	dec	[cnt]
	pop	ecx
	ret

copy_current:
	push	ecx
	mov	edi,[cnt]
	shl	edi,4
	lea	edi,[rct+edi]
	mov	esi,ebp
	mov	ecx,4
	cld
	rep	movsd
	pop	ecx
	inc	[cnt]
	ret

intersect_rect: ; ebp,tr
	mov	rc_top,[tr.top]
	jif	rc_top,ge,[r.bottom],.exit
	mov	rc_right,[tr.right]
	jif	rc_right,le,[r.left],.exit
	mov	rc_bottom,[tr.bottom]
	jif	rc_bottom,le,[r.top],.exit
	mov	rc_left,[tr.left]
	jif	rc_left,ge,[r.right],.exit

	jif	rc_top,ge,[r.top],@f
	mov	rc_top,[r.top]
    @@: jif	rc_right,le,[r.right],@f
	mov	rc_right,[r.right]
    @@: jif	rc_bottom,le,[r.bottom],@f
	mov	rc_bottom,[r.bottom]
    @@: jif	rc_left,ge,[r.left],@f
	mov	rc_left,[r.left]
    @@: clc
	ret
    .exit:
	stc
	ret

endf

func FC
begin
  .x00:
  .x01:
  .x02:
  .x04:
  .x05:
  .x06:
  .x08:
  .x09:
  .x0A:
  .x10:
  .x11:
  .x12:
  .x14:
  .x15:
  .x16:
  .x18:
  .x19:
  .x1A:
  .x20:
  .x21:
  .x22:
  .x24:
  .x25:
  .x26:
  .x28:
  .x29:
  .x2A:
  .x40:
  .x41:
  .x42:
  .x44:
  .x45:
  .x46:
  .x48:
  .x49:
  .x4A:
  .x50:
  .x51:
  .x52:
  .x54:
  .x55:
  .x56:
  .x58:
  .x59:
  .x5A:
  .x60:
  .x61:
  .x62:
  .x64:
  .x65:
  .x66:
  .x68:
  .x69:
  .x6A:
  .x80:
  .x81:
  .x82:
  .x84:
  .x85:
  .x86:
  .x88:
  .x89:
  .x8A:
  .x90:
  .x91:
  .x92:
  .x94:
  .x95:
  .x96:
  .x98:
  .x99:
  .x9A:
  .xA0:
  .xA1:
  .xA2:
  .xA4:
  .xA5:
  .xA6:
  .xA8:
  .xA9:
  .xAA:
	ret
  .xXX:
	ret
endf

;-----------------------------------------------------------------------------
;///// END ///////////////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------