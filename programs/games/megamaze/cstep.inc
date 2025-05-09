; Language support for locales: ru_RU (CP866), en_US.

; level format
; [fx|fy][blue xy][red xy]..[maze walls]

; internal format
; [player]= blue
; [finish]= red


CSTEP_key:
    cmp  eax,176
    jb   .ex
    cmp  eax,179
    ja   .ex
    mov  esi,area
    inc  esi
    lea  ebx,[eax-176]
    mov  [jump],still
    mov  eax,[player]
    call check_move
    jc   .ex
    add  eax,[dirs+ebx*4]
    mov  edx,eax
  .nom1:
    mov  eax,[finish]
    xor  ebx,11b
    call check_move
    jc   .ex
    add  eax,[dirs+ebx*4]
    mov  [finish],eax
    mov  [player],edx
    mov  [jump],drw
    cmp  eax,edx
    jnz  .ex
    mov  [win_flag],1
  .ex:
    ret

CSTEP_drawm:
    mov  eax,[player]
    call get_xy
    mcall 13,[lx],[ly],0xff0000
    mov  eax,[finish]
    call get_xy
    mcall 13,[lx],[ly],0xff
    ret

CSTEP_level:
file 'cstep.bin'

if lang eq ru_RU
        CSTEP_help mstr \
        '�������� ���� � ᨭ�� ����� ��⥬ �ࠢ�����',\
        '����� ���� ������. ����� ���� ᫥��� ��',\
        '����, �� �������� � ��אַ ��⨢���������',\
        '���ࠢ�����. ��� ࠧ�襭 ⮫쪮 � ��砥',\
        '������⢨� �९���⢨� ��� ����� ������.','',\
        'http://www.clickmazes.com'
else ; Default to en_US
        CSTEP_help mstr \
        'Unite the red and blue dots by moving just the',\
        'red dot. The blue dot follows the red dot, but',\
        'moves in exactly in the opposite direction. A',\
        'move is only permitted if the path is clear for',\
        'both dots.','',\
        'http://www.clickmazes.com'
end if
