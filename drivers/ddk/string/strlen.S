# strlen()    Author: Kees J. Bot  1 Jan 1994

# size_t strlen(const char *s)
# Return the length of a string.

.intel_syntax

.globl _strlen

.text

        .align  16
_strlen:
           mov     ecx, -1       # Unlimited length
           jmp     __strnlen     # Common code
