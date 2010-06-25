
if used mmx128_mix_2

align 4
mmx128_mix_2:
           prefetcht1 [eax+128]
           prefetcht1 [ebx+128]

           movaps xmm0, [eax]
           movaps xmm1, [eax+16]
           movaps xmm2, [eax+32]
           movaps xmm3, [eax+48]
           movaps xmm4, [eax+64]
           movaps xmm5, [eax+80]
           movaps xmm6, [eax+96]
           movaps xmm7, [eax+112]

           paddsw xmm0, [ebx]
           movaps [edi], xmm0
           paddsw xmm1,[ebx+16]
           movaps [edi+16], xmm1
           paddsw xmm2, [ebx+32]
           movaps [edi+32], xmm2
           paddsw xmm3, [ebx+48]
           movaps [edi+48], xmm3
           paddsw xmm4, [ebx+64]
           movaps [edi+64], xmm4
           paddsw xmm5, [ebx+80]
           movaps [edi+80], xmm5
           paddsw xmm6, [ebx+96]
           movaps [edi+96], xmm6
           paddsw xmm7, [ebx+112]
           movaps [edi+112], xmm7
           ret

align 4
mmx128_mix_3:
           prefetcht1 [eax+128]
           prefetcht1 [ebx+128]
           prefetcht1 [ecx+128]

           movaps xmm0, [eax]
           movaps xmm1, [eax+16]
           movaps xmm2, [eax+32]
           movaps xmm3, [eax+48]
           movaps xmm4, [eax+64]
           movaps xmm5, [eax+80]
           movaps xmm6, [eax+96]
           movaps xmm7, [eax+112]

           paddsw xmm0, [ebx]
           paddsw xmm1, [ebx+16]
           paddsw xmm2, [ebx+32]
           paddsw xmm3, [ebx+48]
           paddsw xmm4, [ebx+64]
           paddsw xmm5, [ebx+80]
           paddsw xmm6, [ebx+96]
           paddsw xmm7, [ebx+112]

           paddsw xmm0, [ecx]
           movaps [edi], xmm0
           paddsw xmm1, [ecx+16]
           movaps [edi+16], xmm1
           paddsw xmm2, [ecx+32]
           movaps [edi+32], xmm2
           paddsw xmm3, [ecx+48]
           movaps [edi+48], xmm3
           paddsw xmm4, [ecx+64]
           movaps [edi+64], xmm4
           paddsw xmm5, [ecx+80]
           movaps [edi+80], xmm5
           paddsw xmm6, [ecx+96]
           movaps [edi+96], xmm6
           paddsw xmm7, [ecx+112]
           movaps [edi+112], xmm7
           ret

align 4
mmx128_mix_4:
           prefetcht1 [eax+128]
           prefetcht1 [ebx+128]
           prefetcht1 [ecx+128]
           prefetcht1 [edx+128]

           movaps xmm0, [eax]
           movaps xmm2, [eax+16]
           movaps xmm4, [eax+32]
           movaps xmm6, [eax+48]
           movaps xmm1, [ebx]
           movaps xmm3, [ebx+16]
           movaps xmm5, [ebx+32]
           movaps xmm7, [ebx+48]

           paddsw xmm0, [ecx]
           paddsw xmm2, [ecx+16]
           paddsw xmm4, [ecx+32]
           paddsw xmm6, [ecx+48]
           paddsw xmm1, [edx]
           paddsw xmm3, [edx+16]
           paddsw xmm5, [edx+32]
           paddsw xmm7, [edx+48]

           paddsw xmm0, xmm1
           movaps [edi], xmm0
           paddsw xmm2, xmm3
           movaps [edi+16], xmm2
           paddsw xmm4, xmm5
           movaps [edi+32], xmm4
           paddsw xmm6, xmm7
           movaps [edi+48], xmm6

           movaps xmm0, [eax+64]
           movaps xmm2, [eax+80]
           movaps xmm4, [eax+96]
           movaps xmm6, [eax+112]

           movaps xmm1, [ebx+64]
           movaps xmm3, [ebx+80]
           movaps xmm5, [ebx+96]
           movaps xmm7, [ebx+112]
           paddsw xmm0, [ecx+64]
           paddsw xmm2, [ecx+80]
           paddsw xmm4, [ecx+96]
           paddsw xmm6, [ecx+112]

           paddsw xmm1, [edx+64]
           paddsw xmm3, [edx+80]
           paddsw xmm5, [edx+96]
           paddsw xmm7, [edx+112]
           paddsw xmm0, xmm1
           movaps [edi+64], xmm0
           paddsw xmm2, xmm3
           movaps [edi+80], xmm2
           paddsw xmm4, xmm5
           movaps [edi+96], xmm4
           paddsw xmm6, xmm7
           movaps [edi+112], xmm6
           ret
end if
