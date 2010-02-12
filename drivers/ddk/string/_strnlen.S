# _strnlen() Author: Kees J. Bot  1 Jan 1994

# size_t _strnlen(const char *s, size_t ecx)
# Return the length of a string.

.intel_syntax

.globl __strnlen

.text
           .align  16
__strnlen:
           push    ebp
           mov     ebp, esp
           push    edi
           mov     edi, [ebp+8]    # edi = string
           xorb    al, al          # Look for a zero byte
           mov     edx, ecx        # Save maximum count
           cmpb    cl, 1           # 'Z' bit must be clear if ecx = 0
           cld
           repne
           scasb                   # Look for zero
           jne     no0
           inc     ecx             # Don't count zero byte
no0:
           mov     eax, edx
           sub     eax, ecx        # Compute bytes scanned
           pop     edi
           pop     ebp
           ret
