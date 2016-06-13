#define _NEW_TYPE_

#include "tok.h"

void convert_type(int *sign,int *rettype,int *pointr,int reg)
{
int usebracket=FALSE;
	if(tok==tk_openbracket&&(tok2>=tk_char&&tok2<=tk_double)){
		nexttok();
		usebracket=TRUE;
	}
	switch ( tok ){
		case tk_byte:
		case tk_word:
		case tk_dword:
		case tk_float:
		case tk_double:
		case tk_qword:
			*sign=0;
			*rettype=tok;
			if(usebracket)nexttok();
			else getoperand(reg);
			break;
		case tk_char:
		case tk_int:
		case tk_long:
			*sign=1;
			*rettype=tok;
			if(usebracket)nexttok();
			else getoperand(reg);
			break;
	}
	if(usebracket){
		while(tok==tk_mult){
			nexttok();
			*pointr++;
		}
		if(tok!=tk_closebracket)expected(')');
		else getoperand(reg);
	}
}
