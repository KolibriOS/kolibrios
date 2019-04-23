;
;  SNTP library
;
;  (C) 2019 Coldy	
;  Thank's you for use this code and software based on it!
;  I will glad if it's will be helpful. 
;
;  Distributed under terms of GPL
;

; Inline clearing register(s)
; E.g. uses:
; clear eax      ; - clearing single register 
; or
; clear eax, ebx ; ... and so on - clearing multiple register 
macro clear [reg]  { xor reg, reg }

macro b2bcd
;   Convert hex byte to BCD byte
;   Input:
;   al = number 0...99 (not checking)
;   Output:
;   al = number in BCD
;   Algorithm:
;   al = (al > 9) ? ((al / 10) * 6 + al) : al
;   Use registers (not restore):
;   eax, ebx, edx 
{
    clear ebx, edx
    cmp al, 9         ; (al <= 9 ?)  
    jle @f
    mov bl, al
    mov dl, 10
    div dl            ; al = al/10
    mov dl, 6
    mul dl            ; al = al*6
    add al, bl        ; al = al + bl
@@:
}

struct DateTime
  struct
    day   db ?
    month db ?
    year  dw ?
  ends
  struct
    hour  db ?
    min   db ?
    sec   db ?
  ends
ends


proc DateTime2bcd
   ;
   ;  Input:
   ;  eax => pointer to DateTime (UNIX time since 1.1.1970 00:00:00 GMT )
   ;
   ; Output:
   ; eax  => time in ÂÑD format
   ; edx  => date in BCD format
locals
  date dd 0
  time dd 0
endl
  
  mov esi, eax
  clear eax, ebx, ecx
  mov al, [esi + DateTime.day]
  cmp al, 9
  jle @f
  mov bl, al
  mov cl, 10
  div cl
  mul bl, cl
  add al, bl 
@@:
endp



YEAR_EPOCH            = 1970
YEAR_FIRST_LEAP_YEAR  = 1972
SEC_IN_MINUTE         = 60
SEC_IN_HOUR           = (SEC_IN_MINUTE * 60)
SEC_IN_DAY            = (SEC_IN_HOUR * 24)
SEC_IN_YEAR           = (SEC_IN_DAY * 365)
 
proc timestamp2DateTime
   ;
   ;  Input:
   ;  eax => timestamp (UNIX time since 1.1.1970 00:00:00 GMT )
   ;  ebx => pointer to DateTime
   ;
   ; Output:
   ; none
   ;
   ; Use registers (not restore):
   ;
   ; History:
   ; 14.04.2019 Bug fixed: Incorrect output day (-1 day error)!!!
   ; 19.04.2019 Bug fixed: Incorrect convert of time with maximum UNIX time (0x7ffffff)
   ;
   ; Known isuues:
   ;  Not yet seen :) 
   ;
locals
  timestamp  dd ?
  years      dw ?
  lyears     dw ?
  ;year       dw ?
  ;_ts        dd 0
  ;ts         dd 0
  MonthDays  db 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
  ;month      db ?
endl
     
  ;pusha
  mov  [timestamp], eax
  mov  esi, ebx ; ESI => pointer to DateTime
    
  ; Calculate total years since year epoch
  cdq
  mov	  ebx, SEC_IN_YEAR + SEC_IN_DAY/4 
  idiv	ebx
  mov [years], ax ; (AX => years)
  
  ; Calculate current year
  add ax, YEAR_EPOCH
  mov [esi + DateTime.year], ax ; (AX => year)
  
  ; Calculate leap years since year epoch
  ;xor   ecx, ecx ; Clear
  ;mov   cx, bx
  
  ;Fix: -1 day error
  dec   ax
  sub   ax, YEAR_FIRST_LEAP_YEAR ; - 1
  
  clear   ebx, edx
  mov   bx, 4
  idiv  bx
  cmp	  [esi + DateTime.year], YEAR_FIRST_LEAP_YEAR + 1
  js    @f
  inc   al
@@:
  mov [lyears], ax
  
  ; Drop years seconds
  mov   eax, [timestamp]
  clear   ebx
  mov   bx, [years]
  sub   bx, [lyears]
  mov   ecx, SEC_IN_YEAR
  imul  ebx, ecx                  ; ebx => (years - lyears) * SEC_IN_YEAR
  clear   ecx
  mov   cx, [lyears]
  mov   edx, SEC_IN_YEAR + SEC_IN_DAY
  imul  ecx, edx                  ; cx => lyears * (SEC_IN_YEAR + SEC_IN_DAY)
  add   ebx, ecx                  ; bx => (years - lyears) * SEC_IN_YEAR + lyears * (SEC_IN_YEAR + SEC_IN_DAY)
  sub   eax, ebx                
  mov   [timestamp], eax
  
  ;Is leap year?
  clear ecx, edx
  mov   cx, [esi + DateTime.year]
  mov   eax, ecx 
  mov   ebx, 4
  div   bx
  cmp   dx, 0
  je   leap
  mov   ax, cx
  mov  bx, 100
  div  bx
  cmp   dx, 0
  je   leap
  mov   ax, cx
  mov  bx, 400
  clear dx
  div  bx
  cmp   dx, 0
  jmp   @f    
leap:
  ; Add +1 day in february if leap year
  inc [MonthDays + 1]
@@:   
  
  ; Calculate current month
  clear  eax, ecx, edi       ; _ts, ts, month (Bug? => ecx != 0)
caclmonth:
  clear  ebx
  mov  bl, [MonthDays + edi] 
  imul ebx, SEC_IN_DAY
  inc  edi
  add  eax, ebx

  ; {{ 19.4.2019 Bug was somewhere here
  cmp edi, 12
  je    @f
  ; TODO: if edi >= 12 then error!
  ;mov eax, -1
  ;mov edx, 0
  ;ret
;@@:
  cmp eax, [timestamp]
  jge    @f            ; Bug? => jg    @f
  mov  ecx, eax
  jmp  caclmonth
  ; }}
@@:
  ; Drop months seconds
  sub [timestamp], ecx
  mov eax, edi
  mov [esi + DateTime.month], al
  
  ; Calculate elapsed day
  mov   eax, [timestamp]
  clear   edx
  mov   ebx, SEC_IN_DAY
  idiv  ebx     ; eax => day
  
  ; Drop days seconds
  imul  ebx, eax, SEC_IN_DAY
  sub   [timestamp],ebx
  
  ; Correct current day
  add   eax, 1  ; eax => current day
  mov [esi + DateTime.day], al
  
  ; Calculate current hour
  mov   eax, [timestamp]
  clear   edx
  mov   ebx, SEC_IN_HOUR
  idiv  ebx
  mov [esi + DateTime.hour], al
  
  ; Drop hours seconds
  imul ebx, eax, SEC_IN_HOUR
  sub  [timestamp],ebx
  
  mov   eax, [timestamp]
  clear   edx
  mov   ebx, SEC_IN_MINUTE
  idiv  ebx
  mov [esi + DateTime.min], al
  
  ; Drop minutes seconds
  imul ebx, eax, SEC_IN_MINUTE
  sub  [timestamp],ebx
  mov eax, [timestamp]
  mov [esi + DateTime.sec], al
         
  ;popa
  ret
endp