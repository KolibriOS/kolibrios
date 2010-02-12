# strncmp() Author: Kees J. Bot 1 Jan 1994

# int strncmp(const char *s1, const char *s2, size_t n)
#     Compare two strings.
#

.intel_syntax

.globl _strncmp

.text
           .align  16
_strncmp:
           mov     ecx, [esp+12]    # Maximum length
           jmp     __strncmp        # Common code
