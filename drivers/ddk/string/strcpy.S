#       strcpy()                                        Author: Kees J. Bot
#                                                               1 Jan 1994
# char *strcpy(char *s1, const char *s2)
#       Copy string s2 to s1.
#

.intel_syntax

.global _strcpy

        .text
	.align	16
_strcpy:
	push	ebp
	mov	ebp, esp
	push	esi
	push	edi
        mov     ecx, -1         # Unlimited length
        call    _strncpy        # Common code
        mov     eax, [8+ebp]    # Return s1
	pop	edi
	pop	esi
	pop	ebp
	ret
