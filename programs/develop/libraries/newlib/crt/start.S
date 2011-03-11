
.section .init

.global __start

.align 4
__start:
           movl  $68, %eax
           movl  $12, %ebx
           lea  __size_of_stack_reserve__, %ecx
           addl  $4095, %ecx       #load stack size
           andl  $-4096, %ecx      #align to page granularity
           int   $0x40             #and allocate
           testl %eax, %eax
           jz    1f

           addl %eax, %ecx
           movl %eax, %fs:4
           movl %ecx, %fs:8        #save stack base - low limit
                                   #save stack top  - high limit
           movl %ecx, %esp         #reload stack

           subl $1024, %esp

           movl $9, %eax
           movl %esp, %ebx
           movl $-1, %ecx
           int $0x40

           movl 30(%ebx), %eax
           movl %eax, %fs:0        #save pid

           movl $__tls_map, %edi    #init TLS
           movl $32, %ecx
           xorl %eax, %eax
           notl %eax

           rep
           stosl

           movb  $0xF0, __tls_map
           jmp  ___crt_startup
1:
           int3                    #trap to debugger

           .ascii "No enough memory for stack allocation"

