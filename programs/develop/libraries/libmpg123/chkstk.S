
        .global ___chkstk
        .global __alloca

.section .text

___chkstk:
__alloca:
        pushl   %ecx            /* save temp */
        leal    8(%esp), %ecx   /* point past return addr */
        cmpl    $0x1000, %eax   /* > 4k ?*/
        jb      Ldone

Lprobe:
        subl    $0x1000, %ecx           /* yes, move pointer down 4k*/
        orl     $0x0, (%ecx)            /* probe there */
        subl    $0x1000, %eax           /* decrement count */
        cmpl    $0x1000, %eax
        ja      Lprobe                  /* and do it again */

Ldone:
        subl    %eax, %ecx
        orl     $0x0, (%ecx)    /* less than 4k, just peek here */

        movl    %esp, %eax      /* save old stack pointer */
        movl    %ecx, %esp      /* decrement stack */
        movl    (%eax), %ecx    /* recover saved temp */
        movl    4(%eax), %eax   /* recover return address */

        /* Push the return value back.  Doing this instead of just
           jumping to %eax preserves the cached call-return stack
           used by most modern processors.  */
        pushl   %eax
        ret


