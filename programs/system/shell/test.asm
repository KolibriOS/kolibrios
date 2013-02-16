
    use32
    org     0x0

    db	    'MENUET01'	   ; 8 byte id
    dd	    0x01	   ; header version
    dd	    START	   ; start of code
    dd	    I_END	   ; size of image
    dd	    0x10000	   ; memory for app
    dd	    0x10000	   ; esp
    dd	    param_area	   ; I_Param
    dd	    app_path	   ; I_Path

include 'shell.inc'
START:			   ; start of execution

 call _sc_init

 push dword s
 call _sc_gets

 push dword s
 call _sc_puts

 call _sc_exit

 mov eax, -1
 int 0x40


I_END:

param_area	rb  256
app_path	rb  256
s rb 256

