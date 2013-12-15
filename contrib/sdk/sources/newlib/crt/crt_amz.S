

.global __start
.global ___main
.global _DllMainCRTStartup


.section .init

.def	__start;	.scl	2;	.type	32;	.endef 
.def	_DllMainCRTStartup;	.scl	2;	.type	32;	.endef 

.align 4
__start:
_DllMainCRTStartup:

	   call __pei386_runtime_relocator	
           jmp  _main

.align 4
___main:
           ret


