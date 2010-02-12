#      _strncpy()                                      Author: Kees J. Bot
#                                                              1 Jan 1994

# char *_strncpy(char *s1, const char *s2, size_t ecx)
#      Copy string s2 to s1.
#

.intel_syntax

.text
.globl __strncpy
.align  16

__strncpy:
        mov     edi, [ebp+12]   # edi = string s2
        xorb    al, al          # Look for a zero byte
        mov     edx, ecx        # Save maximum count
	cld
	repne
        scasb                   # Look for end of s2
        sub     edx, ecx        # Number of bytes in s2 including null
	xchg	ecx, edx
        mov     esi, [ebp+12]   # esi = string s2
        mov     edi, [ebp+8]     # edi = string s1
	rep
        movsb                   # Copy bytes
	ret
