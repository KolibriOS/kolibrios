
.global ___chkstk
.global __alloca

.section .text

.align 4
___chkstk:
__alloca:
        pushl   %ecx            /* save temp */
        leal    8(%esp), %ecx   /* point past return addr */
        subl    %eax, %ecx
        call *__imp__GetStackBase
        cmpl    %eax, %ecx      # check low stack limit
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
        int3                    #trap to debugger.
                                #Oh wait... there is no kernel debugger
        .ascii "Stack overflow"

