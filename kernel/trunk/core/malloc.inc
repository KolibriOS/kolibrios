$Revision$
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Small heap based on malloc/free/realloc written by Doug Lea
; Version 2.8.3 Thu Sep 22 11:16:15 2005  Doug Lea  (dl at gee)
; Source ftp://gee.cs.oswego.edu/pub/misc/malloc.c
; License http://creativecommons.org/licenses/publicdomain.


; eax= size

; temp
;  esi= nb
;  ebx= idx
;
malloc:
           push esi

; nb = ((size+7)&~7)+8;

           mov esi, eax    ;size
           add esi, 7
           and esi, -8
           add esi, 8

           mov ebx, mst.mutex
           call wait_mutex    ;ebx

           cmp esi, 256
           jae .large

           mov ecx, esi
           shr ecx, 3
           or eax, -1
           shl eax, cl
           and eax, [mst.smallmap]
           jz .small

           push ebp
           push edi

           bsf eax, eax
           mov ebx, eax

; psize= idx<<3;
; B = &ms.smallbins[idx];
; p = B->fd;
; F = p->fd;
; rsize= psize-nb;

           lea ebp, [eax*8]               ;ebp= psize
           shl eax, 4
           lea edi, [mst.smallbins+eax]   ;edi= B
           mov edx, [edi+8]               ;edx= p
           mov eax, [edx+8]               ;eax= F
           mov ecx, ebp
           sub ecx, esi                   ;ecx= rsize

; if (B == F)
           cmp edi, eax
           jne @F

           btr [mst.smallmap], ebx
@@:

; B->fd = F;
; F->bk = B;
; if(rsize<16)

           cmp ecx, 16
           mov [edi+8], eax
           mov [eax+12], edi
           jae .split

; p->head = psize|PINUSE_BIT|CINUSE_BIT;
; (p + psize)->head |= PINUSE_BIT;

           lea eax, [edx+8]
           or dword [edx+ebp+4], 1

           or ebp, 3
           mov [edx+4], ebp

           pop edi
           pop ebp
.done:
           pop esi
           mov [mst.mutex], 0
           ret
.split:
           lea ebx, [edx+8]               ;ebx=mem

; r = chunk_plus_offset(p, nb);
; p->head = nb|PINUSE_BIT|CINUSE_BIT;
; r->head = rsize|PINUSE_BIT;

           lea eax, [edx+esi]             ;eax= r
           or esi, 3
           mov [edx+4], esi

           mov edx, ecx
           or edx, 1
           mov [eax+4], edx

; (r + rsize)->prev_foot = rsize;

           mov [eax+ecx], ecx

; I  = rsize>>3;

           shr ecx, 3

;      ms.smallmap |=  1<< I;
           bts [mst.smallmap], ecx

; B = &ms.smallbins[I];

           shl ecx, 4
           pop edi
           pop ebp
           add ecx, mst.smallbins         ;ecx= B

           mov edx, [ecx+8]               ; F = B->fd;
           mov [ecx+8], eax               ; B->fd = r;
           mov [edx+12], eax              ; F->bk = r;
           mov [eax+8], edx               ; r->fd = F;
           mov [eax+12], ecx              ; r->bk = B;
           mov eax, ebx
           pop esi
           ret
.small:

; if (ms.treemap != 0 && (mem = malloc_small(nb)) != 0)

           cmp [mst.treemap], 0
           je .from_top
           mov eax, esi
           call malloc_small
           test eax, eax
           jz .from_top
           pop esi
           and [mst.mutex], 0
           ret
.large:

; if (ms.treemap != 0 && (mem = malloc_large(nb)) != 0)

           cmp [mst.treemap], 0
           je .from_top

           call malloc_large  ;esi= nb
           test eax, eax
           jne .done
.from_top:

; if (nb < ms.topsize)

           mov eax, [mst.topsize]
           cmp esi, eax
           jae .fail

; rsize = ms.topsize -= nb;
; p = ms.top;

           mov ecx, [mst.top]
           sub eax, esi
           mov [mst.topsize], eax

; r = ms.top = chunk_plus_offset(p, nb);
; r->head = rsize | PINUSE_BIT;
; p->head = nb |PINUSE_BIT|CINUSE_BIT;

           lea edx, [ecx+esi]
           or eax, 1
           mov [mst.top], edx
           or esi, 3
           mov [edx+4], eax
           mov [ecx+4], esi
           lea eax, [ecx+8]
           pop esi
           and [mst.mutex], 0
           ret
.fail:
           xor eax, eax
           pop esi
           and [mst.mutex], 0
           ret

; param
;  eax= mem

align 4
free:
           push edi
           mov edi, eax
           add edi, -8

; if(p->head & CINUSE_BIT)

           test byte [edi+4], 2
           je .fail

           mov ebx, mst.mutex
           call wait_mutex    ;ebx

; psize = p->head & (~3);

           mov eax, [edi+4]
           push esi
           mov esi, eax
           and esi, -4

; next = chunk_plus_offset(p, psize);
; if(!(p->head & PINUSE_BIT))

           test al, 1
           lea ebx, [esi+edi]
           jne .next

; prevsize = p->prev_foot;
; prev=p - prevsize;
; psize += prevsize;
; p = prev;

           mov ecx, [edi]  ;ecx= prevsize
           add esi, ecx              ;esi= psize
           sub edi, ecx              ;edi= p

; if (prevsize < 256)

           cmp ecx, 256
           jae .unlink_large

           mov eax, [edi+8]          ;F = p->fd;
           mov edx, [edi+12]         ;B = p->bk;

; if (F == B)
; ms.smallmap &=  ~(1<< I);
           shr ecx, 3
           cmp eax, edx
           jne @F
           and [mst.smallmap], ecx
@@:
           mov [eax+12], edx           ;F->bk = B;
           mov [edx+8], eax            ;B->fd = F
           jmp .next
.unlink_large:
           mov edx, edi
           call unlink_large_chunk
.next:

; if(next->head & PINUSE_BIT)

           mov eax, [ebx+4]
           test al, 1
           jz .fail2

; if (! (next->head & CINUSE_BIT))

           test al, 2
           jnz .fix_next

; if (next == ms.top)

           cmp ebx, [mst.top]
           jne @F

; tsize = ms.topsize += psize;

           mov eax, [mst.topsize]
           add eax, esi
           mov [mst.topsize], eax

; ms.top = p;
; p->head = tsize | PINUSE_BIT;

           or eax, 1
           mov [mst.top], edi
           mov [edi+4], eax
.fail2:
           and [mst.mutex], 0
           pop esi
.fail:
           pop edi
           ret
@@:

; nsize = next->head & ~INUSE_BITS;

           and eax, -4
           add esi, eax                ;psize += nsize;

; if (nsize < 256)

           cmp eax, 256
           jae .unl_large

           mov edx, [ebx+8]            ;F = next->fd
           mov ebx, [ebx+12]           ;B = next->bk

; if (F == B)

           cmp edx, ebx
           jne @F
           mov ecx, eax
           shr ecx, 3
           btr [mst.smallmap], ecx
@@:
           mov [edx+12], ebx           ;F->bk = B

; p->head = psize|PINUSE_BIT;

           mov ecx, esi
           mov [ebx+8], edx
           or ecx, 1
           mov [edi+4], ecx

; (p+psize)->prev_foot = psize;

           mov [esi+edi], esi

; insert_chunk(p,psize);

           mov eax, esi
           pop esi
           mov ecx, edi
           pop edi
           jmp insert_chunk
.unl_large:

; unlink_large_chunk((tchunkptr)next);

           mov edx, ebx
           call unlink_large_chunk
; p->head = psize|PINUSE_BIT;

           mov ecx, esi
           or ecx, 1
           mov [edi+4], ecx

; (p+psize)->prev_foot = psize;

           mov [esi+edi], esi

; insert_chunk(p,psize);

           mov eax, esi
           pop esi
           mov ecx, edi
           pop edi
           jmp insert_chunk
.fix_next:

; (p+psize)->prev_foot = psize;
; next->head &= ~PINUSE_BIT;
; p->head = psize|PINUSE_BIT;

           and eax, -2
           mov edx, esi
           mov [ebx+4], eax
           or edx, 1
           mov [edi+4], edx

; (p+psize)->prev_foot = psize;

           mov [esi+edi], esi
; insert_chunk(p,psize);

           mov eax, esi
           pop esi
           mov ecx, edi
           pop edi
           jmp insert_chunk

; param
;  ecx = chunk
;  eax = size

align 4
insert_chunk:

           cmp eax, 256
           push esi
           mov esi, ecx
           jae .large

; I  = S>>3;
; ms.smallmap |=  1<< I;

           shr eax, 3
           bts [mst.smallmap], eax

; B = &ms.smallbins[I];

           shl eax, 4
           add eax, mst.smallbins
           mov edx, [eax+8]            ;F = B->fd
           mov [eax+8], esi            ;B->fd = P
           mov [edx+12], esi           ;F->bk = P
           mov [esi+8], edx            ;P->fd = F
           mov [esi+12], eax           ;P->bk = B
           pop esi
           and [mst.mutex], 0
           ret
.large:
           mov ebx, eax
           call insert_large_chunk
           pop esi
           and [mst.mutex], 0
           ret

align 4

; param
;  esi= chunk
;  ebx= size

align 4
insert_large_chunk:

; I = compute_tree_index(S);

           mov edx, ebx
           shr edx, 8
           bsr eax, edx
           lea ecx, [eax+7]
           mov edx, ebx
           shr edx, cl
           and edx, 1
           lea ecx, [edx+eax*2]

; X->index = I;
           mov dword [esi+28], ecx

; X->child[0] = X->child[1] = 0;
           and dword [esi+20], 0
           and dword [esi+16], 0

; H = &ms.treebins[I];

           mov eax, ecx
           lea edx, [mst.treebins+eax*4]

; if (!(ms.treemap & 1<<I))
           bt [mst.treemap], ecx
           jc .tree

; ms.treemap |= 1<<I;
           bts [mst.treemap], ecx
; *H = X;
           mov dword [edx], esi
           jmp .done
.tree:

; T = *H;
           mov edx, [edx]

; K = S << leftshift_for_tree_index(I);
           mov eax, ecx
           shr eax, 1
           sub ecx, 31
           mov edi, 37
           sub edi, eax
           neg ecx
           sbb ecx, ecx
           and ecx, edi
           mov eax, ebx
           shl eax, cl     ;eax= K

           jmp .loop

.not_eq_size:

; C = &(T->child[(K >> 31) & 1]);
           mov ecx, eax
           shr ecx, 31
           lea ecx, [edx+ecx*4+16]

; K <<= 1;
; if (*C != 0)
           mov edi, [ecx]
           add eax, eax
           test edi, edi
           jz .insert_child

; T = *C;
           mov edx, edi
.loop:

; for (;;)
; if ((T->head & ~INUSE_BITS) != S)

           mov ecx, [edx+4]
           and ecx, not 3
           cmp ecx, ebx
           jne .not_eq_size

; F = T->fd;
           mov eax, [edx+8]

; T->fd = F->bk = X;
           mov [eax+12], esi
           mov [edx+8], esi

; X->fd = F;
; X->bk = T;
; X->parent = 0;

           and dword [esi+24], 0
           mov [esi+8], eax
           mov [esi+12], edx
           ret

.insert_child:

; *C = X;
           mov [ecx], esi
.done:

; X->parent = T;
           mov [esi+24], edx

; X->fd = X->bk = X;
           mov [esi+12], esi
           mov [esi+8], esi
           ret


; param
;  edx= chunk

align 4
unlink_large_chunk:

           mov eax, [edx+12]
           cmp eax, edx
           push edi
           mov edi, [edx+24]
           je @F

           mov ecx, [edx+8]           ;F = X->fd
           mov [ecx+12], eax          ;F->bk = R;
           mov [eax+8], ecx           ;R->fd = F
           jmp .parent
@@:
           mov eax, [edx+20]
           test eax, eax
           push esi
           lea esi, [edx+20]
           jne .loop

           mov eax, [edx+16]
           test eax, eax
           lea esi, [edx+16]
           je .l2
.loop:
           cmp dword [eax+20], 0
           lea ecx, [eax+20]
           jne @F

           cmp dword [eax+16], 0
           lea ecx, [eax+16]
           je .l1
@@:
           mov eax, [ecx]
           mov esi, ecx
           jmp .loop
.l1:
           mov dword [esi], 0
.l2:
           pop esi
.parent:
           test edi, edi
           je .done

           mov ecx, [edx+28]
           cmp edx, [mst.treebins+ecx*4]
           lea ecx, [mst.treebins+ecx*4]
           jne .l3

           test eax, eax
           mov [ecx], eax
           jne .l5

           mov ecx, [edx+28]
           btr [mst.treemap], ecx
           pop edi
           ret
.l3:
           cmp [edi+16], edx
           jne @F

           mov [edi+16], eax
           jmp .l4
@@:
           mov [edi+20], eax
.l4:
           test eax, eax
           je .done
.l5:
           mov [eax+24], edi
           mov ecx, [edx+16]
           test ecx, ecx
           je .l6

           mov [eax+16], ecx
           mov [ecx+24], eax
.l6:
           mov edx, [edx+20]
           test edx, edx
           je .done

           mov [eax+20], edx
           mov [edx+24], eax
.done:
           pop edi
           ret

; param
;  esi= nb

align 4
malloc_small:
           push ebp
           mov ebp, esi

           push edi

           bsf eax,[mst.treemap]
           mov ecx, [mst.treebins+eax*4]

; rsize = (t->head & ~INUSE_BITS) - nb;

           mov edi, [ecx+4]
           and edi, -4
           sub edi, esi
.loop:
           mov ebx, ecx
.loop_1:

; while ((t = leftmost_child(t)) != 0)

           mov eax, [ecx+16]
           test eax, eax
           jz @F
           mov ecx, eax
           jmp .l1
@@:
           mov ecx, [ecx+20]
.l1:
           test ecx, ecx
           jz .unlink

; trem = (t->head & ~INUSE_BITS) - nb;

           mov eax, [ecx+4]
           and eax, -4
           sub eax, ebp

; if (trem < rsize)

           cmp eax, edi
           jae .loop_1

; rsize = trem;

           mov edi, eax
           jmp .loop
.unlink:


; r = chunk_plus_offset((mchunkptr)v, nb);
; unlink_large_chunk(v);

           mov edx, ebx
           lea esi, [ebx+ebp]
           call unlink_large_chunk

; if (rsize < 16)

           cmp edi, 16
           jae .split

; v->head = (rsize + nb)|PINUSE_BIT|CINUSE_BIT;

           lea ecx, [edi+ebp]

; (v+rsize + nb)->head |= PINUSE_BIT;

           add edi, ebx
           lea eax, [edi+ebp+4]
           pop edi
           or ecx, 3
           mov [ebx+4], ecx
           or dword [eax], 1
           pop ebp

           lea eax, [ebx+8]
           ret
.split:

; v->head = nb|PINUSE_BIT|CINUSE_BIT;
; r->head = rsize|PINUSE_BIT;
; (r+rsize)->prev_foot = rsize;

           or ebp, 3
           mov edx, edi
           or edx, 1

           cmp edi, 256
           mov [ebx+4], ebp
           mov [esi+4], edx
           mov [esi+edi], edi
           jae .large

           shr edi, 3
           bts [mst.smallmap], edi

           mov eax, edi
           shl eax, 4
           add eax, mst.smallbins

           mov edx, [eax+8]
           mov [eax+8], esi
           mov [edx+12], esi
           pop edi
           mov [esi+12], eax
           mov [esi+8], edx
           pop ebp
           lea eax, [ebx+8]
           ret
.large:
           lea eax, [ebx+8]
           push eax
           mov ebx, edi
           call insert_large_chunk
           pop eax
           pop edi
           pop ebp
           ret


; param
;  esi= nb

align 4
malloc_large:
.idx equ esp+4
.rst equ esp

           push ebp
           push edi
           sub esp, 8
; v = 0;
; rsize = -nb;

           mov edi, esi
           mov ebx, esi
           xor ebp, ebp
           neg edi

; idx = compute_tree_index(nb);

           mov edx, esi
           shr edx, 8
           bsr eax, edx
           lea ecx, [eax+7]
           shr esi, cl
           and esi, 1
           lea ecx, [esi+eax*2]
           mov [.idx], ecx

; if ((t = ms.treebins[idx]) != 0)

           mov eax, [mst.treebins+ecx*4]
           test eax, eax
           jz .l3

; sizebits = nb << leftshift_for_tree_index(idx);

           cmp ecx, 31
           jne @F
           xor ecx, ecx
           jmp .l1
@@:
           mov edx, ecx
           shr edx, 1
           mov ecx, 37
           sub ecx, edx
.l1:
           mov edx, ebx
           shl edx, cl

; rst = 0;
           mov [.rst], ebp
.loop:

; trem = (t->head & ~INUSE_BITS) - nb;

           mov ecx, [eax+4]
           and ecx, -4
           sub ecx, ebx

; if (trem < rsize)

           cmp ecx, edi
           jae @F
; v = t;
; if ((rsize = trem) == 0)

           test ecx, ecx
           mov ebp, eax
           mov edi, ecx
           je .l2
@@:

; rt = t->child[1];

           mov ecx, [eax+20]

; t = t->child[(sizebits >> 31) & 1];

           mov esi, edx
           shr esi, 31

; if (rt != 0 && rt != t)

           test ecx, ecx
           mov eax, [eax+esi*4+16]
           jz @F
           cmp ecx, eax
           jz @F

; rst = rt;
           mov [.rst], ecx
@@:
; if (t == 0)

           test eax, eax
           jz @F

; sizebits <<= 1;

           add edx, edx
           jmp .loop
@@:
; t = rst;
           mov eax, [.rst]
.l2:
; if (t == 0 && v == 0)

           test eax, eax
           jne .l4
           test ebp, ebp
           jne .l7
           mov ecx, [.idx]
.l3:

; leftbits = (-1<<idx) & ms.treemap;
; if (leftbits != 0)

           or edx, -1
           shl edx, cl
           and edx, [mst.treemap]
           jz @F

           bsf eax, edx
; t = ms.treebins[i];
           mov eax, [mst.treebins+eax*4]
@@:

; while (t != 0)
           test eax, eax
           jz .l5
.l4:

; trem = (t->head & ~INUSE_BITS) - nb;

           mov ecx, [eax+4]
           and ecx, -4
           sub ecx, ebx

; if (trem < rsize)

           cmp ecx, edi
           jae @F
; rsize = trem;

           mov edi, ecx
; v = t;
           mov ebp, eax
@@:

; t = leftmost_child(t);

           mov ecx, [eax+16]
           test ecx, ecx
           je @F
           mov eax, ecx
           jmp .l6
@@:
           mov eax, [eax+20]
.l6:

; while (t != 0)

           test eax, eax
           jne .l4
.l5:

; if (v != 0)

           test ebp, ebp
           jz .done
.l7:

; r = chunk_plus_offset((mchunkptr)v, nb);
; unlink_large_chunk(v);

           mov edx, ebp
           lea esi, [ebx+ebp]
           call unlink_large_chunk

; if (rsize < 16)

           cmp edi, 16
           jae .large

; v->head = (rsize + nb)|PINUSE_BIT|CINUSE_BIT;

           lea ecx, [edi+ebx]

; (v+rsize + nb)->head |= PINUSE_BIT;

           add edi, ebp
           lea eax, [edi+ebx+4]
           or ecx, 3
           mov [ebp+4], ecx
           or dword [eax], 1
           lea eax, [ebp+8]
           add esp, 8
           pop edi
           pop ebp
           ret
.large:

; v->head = nb|PINUSE_BIT|CINUSE_BIT;
; r->head = rsize|PINUSE_BIT;

           mov edx, edi
           or ebx, 3
           mov [ebp+4], ebx
           or edx, 1
           mov [esi+4], edx

; (r+rsize)->prev_foot = rsize;
; insert_large_chunk((tchunkptr)r, rsize);

           mov [esi+edi], edi
           mov eax, edi
           mov ecx, esi
           call insert_chunk

           lea eax, [ebp+8]
           add esp, 8
           pop edi
           pop ebp
           ret
.done:
           add esp, 8
           pop edi
           pop ebp
           xor eax, eax
           ret

align 4
init_malloc:

           stdcall kernel_alloc, 0x20000

           mov [mst.top], eax
           mov [mst.topsize], 128*1024
           mov dword [eax+4], (128*1024) or 1
           mov eax, mst.smallbins
@@:
           mov [eax+8], eax
           mov [eax+12], eax
           add eax, 16
           cmp eax, mst.smallbins+512
           jl @B

           ret




