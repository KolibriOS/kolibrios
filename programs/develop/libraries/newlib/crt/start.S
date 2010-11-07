
.section .init

.global __start
.global __exit
.global __Exit

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
           movl %ecx, %esp
           jmp  ___crt_startup     #reload stack
1:
           int3                    #trap to debugger

           .ascii "No enough memory for stack allocation"

.align 4
__exit:
__Exit:
           movl  4(%esp), %edx     #store exit code
           movl  $68, %eax
           movl  $13, %ebx
           movl  %fs:4, %ecx
           int   $0x40             #destroy stack

           movl  $-1, %eax
           int   $0x40             #terminate thread
