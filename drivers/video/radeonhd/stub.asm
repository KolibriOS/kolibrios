        format  MZ
	heap	0
        stack   800h
	entry	main:start

segment main use16

start:
           push    cs
           pop     ds

           mov     dx, msg
           mov     ah, 9
           int     21h             ; DOS - PRINT STRING
                                   ; DS:DX -> string terminated by "$"
           mov     ax, 4C01h
           int     21h             ; DOS - 2+ - QUIT WITH EXIT CODE (EXIT)
                                   ; AL = exit code

 ; ---------------------------------------------------------------------------
 msg db 'This is Kolibri OS device driver.',0Dh,0Ah,'$',0

