;
;   (c) 2022 Coldy
;
;   This module tiny version of crt0.asm
;   (for linking tiny app, incl. non C modules)
;

format ELF
 
    
;==============================
;public argc as '__argc' - no needed for tiny app
public params as '__argv'
public path as '__path'

section '.bss' 
buf_len = 0x400
path     rb buf_len 
params   rb buf_len 

;section '.data'
