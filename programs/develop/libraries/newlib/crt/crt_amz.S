
.section .text

.global __start
.global ___main

.align 4
__start:
	   call __pei386_runtime_relocator	
           jmp  _main

.align 4
___main:
           ret


