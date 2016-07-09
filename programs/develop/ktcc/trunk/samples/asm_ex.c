/* examples for interoperability with assembler

1. Calling assembler code from .c : see in libc\math any .asm file
2. Using inline assembler: see \include\kos32sys1.h and libc\math\fmod.c
- https://gcc.gnu.org/onlinedocs/gcc-4.8.5/gcc/Extended-Asm.html
- not all constraints from gcc are supported, no "f" or "t" for example
- not supported clobberring st registers, must manual add "fstp %%st" at end or similar
- need full suffixes for opcodes, fstpl but not fstp

3. Calling c functions from .asm: see \libc\start\start.asm:99
Remember: 
- small ints always passed as int32, floating point params as 64-bit
- returned structs passed on stack with additional hidden 1st param
- c functions can use EAX, ECX, EDX without warnings
- .c default is cdecl calling convention https://en.wikipedia.org/wiki/X86_calling_conventions 
- dont use fastcall calling convention, tinycc realized it non-conformant way
- tinycc supports only ELF object files

tcc can be used as a linker
*/




#include <stdio.h>
#include <math.h>


main()
{	
	int i;
	for (i = 0; i < 20; i++)
	{
	printf("------------------------------------------------------\n");
	printf ( "remainder of 5.3 / 2 is %f\n", remainder (5.3,2) );
  	printf ( "remainder of 18.5 / 4.2 is %f\n", remainder (18.5,4.2) );
//remainder of 5.3 / 2 is -0.700000
//remainder of 18.5 / 4.2 is 1.700000

	printf ( "fmod of 5.3 / 2 is %f\n", fmod (5.3,2) );
	printf ( "fmod of 18.5 / 4.2 is %f\n", fmod (18.5,4.2) );
// fmod of 5.3 / 2 is 1.300000
// fmod of 18.5 / 4.2 is 1.700000

	double param, fractpart, intpart, result;
	int n;

  	param = 3.14159265;
  	fractpart = modf (param , &intpart);
  	printf ("%f = %f + %f \n", param, intpart, fractpart);
//3.141593 = 3.000000 + 0.141593

 	param = 0.95;
	n = 4;
	result = ldexp (param , n);
	printf ("%f * 2^%d = %f\n", param, n, result);
//0.950000 * 2^4 = 15.200000

	param = 8.0;
	result = frexp (param , &n);
	printf ("%f = %f * 2^%d\n", param, result, n);
//8.000000 = 0.500000 * 2^4
	param = 50;
	result = frexp (param , &n);
	printf ("%f = %f * 2^%d\n", param, result, n);

	}    
}