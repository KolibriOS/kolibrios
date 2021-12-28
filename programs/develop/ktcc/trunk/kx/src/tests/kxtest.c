//
// This sample show that KX extension supported or not

#include <stdio.h>

int main(){

  char* kx_ext =
#ifndef __KX__
	"not "
#endif
	 "supported"
	 ;
   
  printf("KX extension %s", kx_ext);
  
  return 0;
}