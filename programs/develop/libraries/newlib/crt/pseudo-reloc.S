
.global __pei386_runtime_relocator

.text

__pei386_runtime_relocator:

#    movl  $___RUNTIME_PSEUDO_RELOC_LIST__, %ecx

#    pushl  %ebp
#    cmpl  $___RUNTIME_PSEUDO_RELOC_LIST_END__, %ecx
#    movl %esp, %ebp
#    jnb .L2

#.L1:
#    movl (%ecx), %eax
#    movl 4(%ecx), %edx
#    addl  $8, %ecx
#    addl %eax, __image_base__(%edx)
#    cmpl  $___RUNTIME_PSEUDO_RELOC_LIST_END__, %ecx
#    jb  .L1

#.L2:
#    popl  %ebp
    ret

