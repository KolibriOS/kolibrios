#include <stdio.h>

#	define TRACE1(s, a) printf(s, a)

void caller(void* ptr)
{

   ptr = 0xaaaaaaaa;
   TRACE1("caller is called from EIP@%x\n", *(int*)((char*)&ptr-4)-5);

}


int main()
{
	
	caller(0xffffffff);
}

