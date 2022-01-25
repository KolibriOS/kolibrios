;
;  String helpers
;
;  (C) KolibriOS team (parts from another project)
;  (C) 2022 Coldy (str_buld function)	
;  Thank's you for use this code and software based on it!
;  I will glad if it's will be helpful. 
;
;  Distributed under terms of GPL
;

;****************************************
;*  input:  esi = pointer to string     *
;*  output: ecx = length of the string  *
;****************************************
strlen:
      push	eax esi
      xor	ecx, ecx
      @@:
	lodsb
	or	al, al
	jz	@f
	inc	ecx
	jmp	@b
      @@:
      pop      esi eax
  ret
  
;*************************************************
;*  input:  esi = pointer to the src string      *
;*          edi = pointer to the dest string     *
;*          ecx = number of bytes to copy        *
;*************************************************
strncpy:
      push	eax ecx esi edi
      @@:
	lodsb
	stosb
	or	al, al
	jz	@f
	dec	ecx
	jz	@f
	jmp	@b
      @@:
      pop	edi esi ecx eax
  ret
 
if 0 ; { Not used  
  
;*************************************************
;*  input:  esi = pointer to the src string      *
;*          edi = pointer to the dest string     *
;*************************************************
strcpy:
     push  esi edi
     ; ecx = ???
     ; ZF = 0
     rep   movsb
     pop   edi esi
  ret
  
;*************************************************
;*  input:  esi = pointer to the src string      *
;*          edi = pointer to the dest string     *
;*          ecx = number of bytes to copy        *
;*************************************************
strncat:
     push  edi
     push  ecx esi
     mov   esi, edi
     call  strlen
     add   edi, ecx
     pop   esi ecx
     call  strncpy
     pop   edi
  ret
  
;*************************************************
;*  (c) Coldy 2022                               *
;*  input:  edi = pointer to the dest string     *
;*          ecx = number of bytes to zero        *
;*************************************************
;memnz:
;     push  eax ecx edi
;     xor   eax, eax
;     rep   stosb
;     pop   edi ecx eax 
;  ret

end if ; }

;
; str_build
;
; Purose:       Build output string by template. Allocate necessary output
;               buffer, copy parts from template and insert strings instead
;               of $ wildcard.
;               
;               SPECIAL CASE:
;               For use dollar sing ($) in text, just mark this plase(s) in
;               template and provide pointer(s) on string with this sign in args)
;
;               PRECAUTION: 
;            1. Not safe, caller must provide args count >= $ wildcard count 
;            2. If used dynamic memory allocator then caller must free output
;               buffer
;            3. Looks like cdecl, but she is not. For cdecl need compat wrapper
;            4. Dirties all registers, incl. ebp
;
; Input: 
;       esp+4 = pointer to template string
;       esp+8 = wildcard strings pointers in reverse order
;
; Options:
if ~STR_BUILD_OFFSET 
STR_BUILD_OFFSET = 0
;               Optional, specify STR_BUILD_OFFSET value for offset from start
;               of output buffer (this useful for postinsert initial characters
;               before output sting). By default - no offset (0)
end if
if ~STR_BUILD_EXTRA
STR_BUILD_EXTRA = 0
;               Optional, specify STR_BUILD_EXTRA value for extra length of
;               output bufer (this useful for postadding characters after
;               output string). By default - no extra length (0)
end if

;
; { STR_BUILD_NO_DOLLAR_SIGN - should be removed, see cpecial case above } 

;
;              Next two options below can reduse total code size by exclude
;              corresponding parts if they are not used
;
if ~STR_BUILD_NO_STARTING_TEXT
STR_BUILD_NO_STARTING_TEXT  = 0
;              Specify STR_BUILD_NO_STARTING_TEXT if you do not used templates
;              starting with text, e.g."Some text first $, $"
;              By default is disabled (0)
end if
;
if ~STR_BUILD_NO_DOUBLE_WILDCARD
STR_BUILD_NO_DOUBLE_WILDCARD  = 0
;              Specify STR_BUILD_NO_DOUBLE_WILDCARD if you not used templates
;              with double wildcards, e.g. "$$ some text" or "Some text $$"
;              By default is disabled (0)
end if 
;
;   mem_alloc(size)
;               external memory allocator, stdcall. Must return pointer (in eax)
;               to base of memory block by size length. By defauld used internal
;
; Output: 
;         eax = Pointer to output string (see PRECAUTION #2) or 0 if error
;         edi = Cursor of output string. No mean inf if eax = 0    
;
; Stack struct
;         ------
;        | ArgN  | 
;         -------
;        |  ...  |
;         -------
;  ebp+4 | Arg1  |
;  ------ -------
;  ebp   | TplS  |  Template string
;  ------ -------
;        | ret   |  Caller return address (not used), esp when entry 
;         -------
;        | $off1 |  1st offset in template string 
;         -------
;        |  ...  |
;         -------
;        | $offN |  N-offset in template string
;         -------
;        | EOTpl |  End of template string, esp after phase 1.1
;         -------
;        | Len1  |  Length of 1st wildcard string  
;         -------
;        |  ...  | 
;         -------
;        | LenN  |  Length of N wildcard string, esp after phase 1.2  
;         ------- 
;
str_build:
     mov  ebp, esp        ; Store caller esp...
     add  ebp, 8          ; ... and shift return address and tamplate string 
     mov  esi, [ebp-4]   
     xor	edx, edx        ; Offsets, length... 
     xor	edi, edi        ; Found count...
     
     ; Phase 1.1. Scan to find positions $ and store to stack offsets $+1
     ; and end of template string. Break scan if zero byte appear
.scan:
     lodsb
     inc	edx
     or	al, al
     jz	.end_scan
     cmp  al, '$'
     je	.found
     jmp	.scan
.found:  
     push   edx           ; Store offset
     inc    edi
     jmp    .scan 
.end_scan:
    or      edi, edi
    jz      .error        ; Not found
    push    edx           ; Store last offset (end of template string) 
    sub     edx, edi      
    dec     edx           ; Total length + zero string             
    
    ; Phase 1.2. Store to stack lengths of wildcard strings
    mov    eax, edi
@@: 
    mov    esi,[ebp+4*(eax-1)]
    call   strlen
    add    edx, ecx
    push   ecx
    dec    eax
    inc    edi                      ; once edi*2 instead
    test   eax,eax
    jnz    @b
    
    add     edx, STR_BUILD_OFFSET + STR_BUILD_EXTRA  
    
    ; Phase 1.3. Allocate buffer for output string
if defined mem_alloc
    stdcall mem_alloc, edx
else
    mov    eax, 68
    mov    ebx, 12
    mov    ecx, edx
    int    0x40
end if 
    test   eax,eax
    jz     .exit
    
    mov    byte[eax+edx],0          ; Mark end of output string
    
    ; Phase 2. Build output string
    
                                    ; eax = base of output string 
    xor    ebx, ebx                 ; ebx = index of args data 
                                    ; ecx = free, but used below
    mov   edx, edi                  ; edx = index of stored data
                                    ; esi = free, but used below 
    mov   edi, eax                  ; edi = cursor of output string
    
    add   edi, STR_BUILD_OFFSET
if ~STR_BUILD_NO_STARTING_TEXT ; {    
    mov   ecx, [esp+4*edx]          ; Offset
    cmp   ecx,1                     ; Wildcard first?
    je    .build

    mov   esi, -2                   ; One or double wildcard at the end
    neg   ecx
    add   ecx, [esp+4*edx-4]        ; Next offset
    cmp   ecx, 1                    ; More one wildcard at the end? 
    je    @f
    dec  esi                         
        
@@:
    mov   ecx,esi
    add   ecx,[esp+4*edx-4]         ; Next offset
    mov   esi,[ebp-4]               ; Template string
    call   strncpy
    add    edi, ecx                 ; Advance cursor
end if; } STR_BUILD_NO_STARTING_TEXT    
.build:
    mov   esi, [ebp+4*ebx]          ; Wildcard string
    mov   ecx,[esp+4*ebx]           ; Length
    call   strncpy
    add    edi, ecx                 ; Advance cursor
    mov   ecx, [esp+4*edx]          ; Offset      

    mov   esi,[ebp-4]               ; Template string
    add   esi, ecx
    cmp  byte [esi], 0              ; End of string?
    je    .exit
if ~STR_BUILD_NO_DOUBLE_WILDCARD ; {   
    cmp  byte [esi], '$'      
    je   @f
end if; } STR_BUILD_NO_DOUBLE_WILDCARD
    neg   ecx
    add   ecx,[esp+4*edx-4]         ; Next offset
    dec   ecx
    call   strncpy
    add    edi, ecx                 ; Advance cursor
@@: ; {  STR_BUILD_NO_DOUBLE_WILDCARD }  
    inc ebx
    dec edx
    cmp ebx, edx
    jne .build
.exit:
    ; Restore stack
    sub   ebp, 8
    mov   esp,ebp         
    ret
    
.error:
    xor   eax, eax
    ret     

