
.global _create_thread
.global ___thread_startup

.section .text

.align 4
_create_thread:
#.thr_proc    equ esp+8
#.param       equ esp+12
#.stack_size  equ esp+16

           pushl %ebx

           movl  $68, %eax
           movl  $12, %ebx
           movl  16(%esp), %ecx    #[.stack_size]
           addl  $4095, %ecx
           andl  $-4096, %ecx
           movl  %ecx, 16(%esp)    #save stack size
           int   $0x40
           testl %eax, %eax
           jz    1f

           leal  -20(%eax,%ecx), %edx

           movl  8(%esp), %ebx     #[.thr_proc]
           mov  %ebx, 4(%edx)

           movl  12(%esp), %ebx    #[.param]
           movl  %ebx, 8(%edx)

           addl  %eax, %ecx
           movl  %eax, 12(%edx)    #stack low limit
           movl  %ecx, 16(%edx)    #stack high limit

           movl  $51, %eax
           movl  $1,  %ebx
           lea   ___thread_startup , %ecx
           int   $0x40
           popl  %ebx
           ret
1:
           notl %eax
           popl %ebx
           ret
