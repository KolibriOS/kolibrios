

.section .text

.global __exit
.global __Exit


.align 4
__exit:
__Exit:
           movl  4(%esp), %edx     #store exit code
           movl  $68, %eax
           movl  $13, %ebx
           movl  %fs:8, %ecx
           int   $0x40             #destroy stack

           movl  $-1, %eax
           int   $0x40             #terminate thread

