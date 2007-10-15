;eax - size
;OUT
;eax - pointer
malloc:

      push ebx
      push ecx

      mov ecx,eax
      mov eax,68
      mov ebx,12
      mcall

      pop ecx
      pop ebx

      ret

;eax - pointer
free:
      push ebx
      push ecx

      mov ecx,eax
      mov eax,68
      mov ebx,13
      mcall

      pop ecx
      pop ebx

      ret