;
;	Kolibri-A Special service template  --  art_zh 2011
;
;	ah = syscall function# (default: 255) 
;	al = subfunction
; 

align 4
special_srv:
	mov	cl, ah
	and	cl, 3
	call	dword[spec_servetable + ecx*4]
	ret

align 4
spec_servetable:
	dd	paleholder	; set something special here
	dd	paleholder
	dd	paleholder
	dd	paleholder