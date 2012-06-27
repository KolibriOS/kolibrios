
.global ___chkstk
.global ___chkstk_ms
.global __alloca

.section .text

#.def	___chkstk;	.scl	2;	.type	32;	.endef
#.def ___chkstk_ms;  .scl  2;  .type 32; .endef
#.def __alloca; .scl  2;  .type 32; .endef

___chkstk:
__alloca:
        pushl   %ecx            /* save temp */
        leal    8(%esp), %ecx   /* point past return addr */
        subl    %eax, %ecx
        cmpl    %fs:4, %ecx     # check low stack limit
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


___chkstk_ms:
       pushl  %ecx            /* save temp */
       pushl  %eax
       cmpl $0x1000, %eax     /* > 4k ?*/
       leal 12(%esp), %ecx    /* point past return addr */
       jb 2f

1:
       subl $0x1000, %ecx     /* yes, move pointer down 4k*/
       cmpl %fs:4, %ecx       /* check low stack limit    */
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
