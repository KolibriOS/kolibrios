
.section .init

.global __start

#tls:0  pid process id
#tls:4  tid reserved for thread slot
#tls:8  thread's stack low limit
#tls:12 thread's stack high limit
#tls:16 reseved for libc

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
           movl %eax, %fs:8
           movl %ecx, %fs:12       #save stack base - low limit
                                   #save stack top  - high limit
           movl %ecx, %esp         #reload stack

           subl $1024, %esp

           movl $9, %eax
           movl %esp, %ebx
           movl $-1, %ecx
           int $0x40

           movl 30(%ebx), %eax
           movl %eax, %fs:0        #save pid

           addl $1024, %esp

           jmp  ___crt_startup
1:
           int3                    #trap to debugger

           .ascii "No enough memory for stack allocation"

