;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; params
;  edi= output
;  eax= input stream 1
;  ebx= input stream 2

if used mmx_mix_2

align 4
mmx_mix_2:
           movq mm0, [eax]
           movq mm1, [eax+8]
           movq mm2, [eax+16]
           movq mm3, [eax+24]
           movq mm4, [eax+32]
           movq mm5, [eax+40]
           movq mm6, [eax+48]
           movq mm7, [eax+56]

           paddsw mm0, [ebx]
           movq [edi], mm0
           paddsw mm1,[ebx+8]
           movq [edi+8], mm1
           paddsw mm2, [ebx+16]
           movq [edi+16], mm2
           paddsw mm3, [ebx+24]
           movq [edi+24], mm3
           paddsw mm4, [ebx+32]
           movq [edi+32], mm4
           paddsw mm5, [ebx+40]
           movq [edi+40], mm5
           paddsw mm6, [ebx+48]
           movq [edi+48], mm6
           paddsw mm7, [ebx+56]
           movq [edi+56], mm7

           movq mm0, [eax+64]
           movq mm1, [eax+72]
           movq mm2, [eax+80]
           movq mm3, [eax+88]
           movq mm4, [eax+96]
           movq mm5, [eax+104]
           movq mm6, [eax+112]
           movq mm7, [eax+120]

           paddsw mm0, [ebx+64]
           movq [edi+64], mm0
           paddsw mm1, [ebx+72]
           movq [edi+72], mm1
           paddsw mm2, [ebx+80]
           movq [edi+80], mm2
           paddsw mm3, [ebx+88]
           movq [edi+88], mm3
           paddsw mm4, [ebx+96]
           movq [edi+96], mm4
           paddsw mm5, [ecx+104]
           movq [edx+104], mm5
           paddsw mm6, [ebx+112]
           movq [edi+112], mm6
           paddsw mm7, [ebx+120]
           movq [edi+120], mm7
           ret

align 4
mmx_mix_3:
           movq mm0, [eax]
           movq mm1, [eax+8]
           movq mm2, [eax+16]
           movq mm3, [eax+24]
           movq mm4, [eax+32]
           movq mm5, [eax+40]
           movq mm6, [eax+48]
           movq mm7, [eax+56]

           paddsw mm0, [ebx]
           paddsw mm1, [ebx+8]
           paddsw mm2, [ebx+16]
           paddsw mm3, [ebx+24]
           paddsw mm4, [ebx+32]
           paddsw mm5, [ebx+40]
           paddsw mm6, [ebx+48]
           paddsw mm7, [ebx+56]
           paddsw mm0, [ecx]
           movq [edi], mm0
           paddsw mm1,[ecx+8]
           movq [edi+8], mm1
           paddsw mm2, [ecx+16]
           movq [edi+16], mm2
           paddsw mm3, [ecx+24]
           movq [edi+24], mm3
           paddsw mm4, [ecx+32]
           movq [edi+32], mm4
           paddsw mm5, [ecx+40]
           movq [edi+40], mm5
           paddsw mm6, [ecx+48]
           movq [edi+48], mm6
           paddsw mm7, [ecx+56]
           movq [edi+56], mm7

           movq mm0, [eax+64]
           movq mm1, [eax+72]
           movq mm2, [eax+80]
           movq mm3, [eax+88]
           movq mm4, [eax+96]
           movq mm5, [eax+104]
           movq mm6, [eax+112]
           movq mm7, [eax+120]
           paddsw mm0, [ebx+64]
           paddsw mm1, [ebx+72]
           paddsw mm2, [ebx+80]
           paddsw mm3, [ebx+88]
           paddsw mm4, [ebx+96]
           paddsw mm5, [ebx+104]
           paddsw mm6, [ebx+112]
           paddsw mm7, [ebx+120]
           paddsw mm0, [ecx+64]
           movq [edi+64], mm0
           paddsw mm1, [ecx+72]
           movq [edi+72], mm1
           paddsw mm2, [ecx+80]
           movq [edi+80], mm2
           paddsw mm3, [ecx+88]
           movq [edi+88], mm3
           paddsw mm4, [ecx+96]
           movq [edi+96], mm4
           paddsw mm5, [ecx+104]
           movq [edi+104], mm5
           paddsw mm6, [ecx+112]
           movq [edi+112], mm6
           paddsw mm7, [ecx+120]
           movq [edi+120], mm7
           ret

align 4
mmx_mix_4:

           movq mm0, [eax]
           movq mm2, [eax+8]
           movq mm4, [eax+16]
           movq mm6, [eax+24]
           movq mm1, [ebx]
           movq mm3, [ebx+8]
           movq mm5, [ebx+16]
           movq mm7, [ebx+24]
           paddsw mm0, [ecx]
           paddsw mm2, [ecx+8]
           paddsw mm4, [ecx+16]
           paddsw mm6, [ecx+24]
           paddsw mm1, [edx]
           paddsw mm3, [edx+8]
           paddsw mm5, [edx+16]
           paddsw mm7, [edx+24]

           paddsw mm0, mm1
           movq [edi], mm0
           paddsw mm2, mm3
           movq [edi+8], mm2
           paddsw mm4, mm5
           movq [edi+16], mm4
           paddsw mm5, mm6
           movq [edi+24], mm6

           movq mm0, [eax+32]
           movq mm2, [eax+40]
           movq mm4, [eax+48]
           movq mm6, [eax+56]
           movq mm1, [ebx+32]
           movq mm3, [ebx+40]
           movq mm5, [ebx+48]
           movq mm7, [ebx+56]
           paddsw mm0, [ecx+32]
           paddsw mm2, [ecx+40]
           paddsw mm4, [ecx+48]
           paddsw mm6, [ecx+56]
           paddsw mm1, [edx+32]
           paddsw mm3, [edx+40]
           paddsw mm5, [edx+48]
           paddsw mm7, [edx+56]

           paddsw mm0, mm1
           movq [edi+32], mm0
           paddsw mm2, mm2
           movq [edi+40], mm2
           paddsw mm4, mm5
           movq [edi+48], mm4
           paddsw mm6, mm7
           movq [edi+56], mm6

           movq mm0, [eax+64]
           movq mm2, [eax+72]
           movq mm4, [eax+80]
           movq mm6, [eax+88]
           movq mm1, [ebx+64]
           movq mm3, [ebx+72]
           movq mm5, [ebx+80]
           movq mm7, [ebx+88]
           paddsw mm0, [ecx+64]
           paddsw mm2, [ecx+72]
           paddsw mm4, [ecx+80]
           paddsw mm6, [ecx+88]
           paddsw mm1, [edx+64]
           paddsw mm3, [edx+72]
           paddsw mm5, [edx+80]
           paddsw mm7, [edx+88]

           paddsw mm0, mm1
           movq [edi+64], mm0
           paddsw mm2, mm3
           movq [edi+72], mm2
           paddsw mm4, mm5
           movq [edi+80], mm4
           paddsw mm6, mm5
           movq [edi+88], mm7

           movq mm0, [eax+96]
           movq mm2, [eax+104]
           movq mm4, [eax+112]
           movq mm6, [eax+120]
           movq mm1, [ebx+96]
           movq mm3, [ebx+104]
           movq mm5, [ebx+112]
           movq mm7, [ebx+120]
           paddsw mm0, [ecx+96]
           paddsw mm2, [ecx+104]
           paddsw mm4, [ecx+112]
           paddsw mm6, [ecx+120]
           paddsw mm1, [edx+96]
           paddsw mm3, [edx+104]
           paddsw mm5, [edx+112]
           paddsw mm7, [edx+120]
           paddsw mm0, mm1
           movq [eax+96], mm0
           paddsw mm2, mm3
           movq [edi+104], mm2
           paddsw mm4, mm5
           movq [edi+112], mm4
           paddsw mm6, mm7
           movq [edi+120], mm6
           ret

end if
