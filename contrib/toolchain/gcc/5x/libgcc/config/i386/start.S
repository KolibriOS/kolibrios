
#tls:0  pid process id
#tls:4  tid reserved for thread slot
#tls:8  thread's stack low limit
#tls:12 thread's stack high limit
#tls:16 reseved for libc

#ifdef L_start

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

#endif


#ifdef L_chkstk

          .section .text
          .global ___chkstk
          .global __alloca

___chkstk:
__alloca:
          pushl   %ecx            /* save temp */
          leal    8(%esp), %ecx   /* point past return addr */
          subl    %eax, %ecx
          cmpl    %fs:8, %ecx     # check low stack limit
          jb      1f

          movl    %esp, %eax      /* save old stack pointer */
          movl    %ecx, %esp      /* decrement stack */
          movl    (%eax), %ecx    /* recover saved temp */
          movl    4(%eax), %eax   /* recover return address */

        /* Push the return value back.  Doing this instead of just
           jumping to %eax preserves the cached call-return stack
           used by most modern processors.  */
          pushl   %eax
          ret
1:
          int3                    #trap to debugger
          .ascii "Stack overflow"
#endif

#ifdef L_chkstk_ms

          .section .text
          .global ___chkstk_ms

___chkstk_ms:
          pushl  %ecx            /* save temp */
          pushl  %eax
          cmpl $0x1000, %eax     /* > 4k ?*/
          leal 12(%esp), %ecx    /* point past return addr */
          jb 2f
1:
          subl $0x1000, %ecx     /* yes, move pointer down 4k*/
          cmpl %fs:8, %ecx       /* check low stack limit    */
          jb 3f

          orl $0x0, (%ecx)       /* probe there */
          subl $0x1000, %eax     /* decrement count */
          cmpl $0x1000, %eax
          ja 1b                  /* and do it again */

2:
          subl %eax, %ecx
          orl $0x0, (%ecx) /* less than 4k, just peek here */

          popl %eax
          popl %ecx
          ret
3:
          int3                    #trap to debugger
          .ascii "Stack overflow"
#endif

