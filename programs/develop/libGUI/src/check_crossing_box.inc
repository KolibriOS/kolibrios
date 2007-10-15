;eax - x1
;ebx - y1
;ecx - size x
;edx - size y
;esi - x2
;edi - y2
;if crossing than eax=0xffffff

CheckCrossingBox:

      add ecx,eax
      add edx,ebx

      cmp esi,eax
      jl exit_crossing
      cmp esi,ecx
      ja exit_crossing
      cmp edi,ebx
      jl exit_crossing
      cmp edi,edx
      ja exit_crossing
      xor eax,eax
      mov eax,0xffffff
      exit_crossing:

      ret
