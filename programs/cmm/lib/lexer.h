#ifndef INCLUDE_LEXER_H
#define INCLUDE_LEXER_H

#ifndef INCLUDE_STRING_H
#include "../lib/strings.h"
#endif
/** Splits text into tokens
 *  Author  :Pavel Yakovlev
 *  Homepage:https://vk.com/pavelyakov39
 */

/** Example:
 *  lexer lex;
 *  lex.load("var a=123;");
 *  lex.next();
 *  lex.token; //TOKEN == 'var'
 *  lex.type ; //TYPE  == LEX_VAR
 *  
 *  lex.next();
 *  lex.token; //TOKEN == 'a'
 *  lex.type ; //TYPE  == LEX_VAR
 *  
 *  lex.next();
 *  lex.token; //TOKEN == '='
 *  lex.type ; //TYPE  == LEX_IND
 *  
 *  lex.next();
 *  lex.token; //TOKEN == '123'
 *  lex.type ; //TYPE  == LEX_DEC
 *  
 *  lex.next();
 *  lex.token; //TOKEN == ';'
 *  lex.type ; //TYPE  == LEX_IND
 *  
 *  lex.next();
 *  lex.token; //TOKEN == ''
 *  lex.type ; //TYPE  == LEX_END
 */

#define LEX_END 1
#define LEX_STR 2
#define LEX_DEC 3
#define LEX_VAR 4
#define LEX_FNC 5
#define LEX_IND 6
#define LEX_NUL 0

:char const_token_lexer[1024];
:struct lexer
{
	byte cmd;
	dword token,text;
	byte type;
	char quote;
	signed count,length;
	dword next(void);
	dword back(void);
	void load(dword _text);
	void expected(dword _text);
};


:void expected(dword _text)
{
	notify(_text);
	ExitProcess();
}

:void lexer::load(dword _text)
{
	text = _text;
}

:dword lexer::next(void)
{
	char s;
	dword pos,in;
	pos = #const_token_lexer;
	in = text;
	
	NEXT_TOKEN:
	length = 0;
	loop()
	{
		s = DSBYTE[in];
		if(s!=9)&&(s!=10)&&(s!=13)&&(s!=32)break;
		in++;
		text++;
	}
	
	if(s==0){type=LEX_END;DSBYTE[pos]=0;token="";return token;}
	
	if(s=='/')
	{
		in++;
		s = DSBYTE[in];
		
		// Line comments
		if(s=='/')
		{
			loop()
			{
				in++;
				s = DSBYTE[in];
				if(s==10)||(s==13)||(s==0)goto NEXT_TOKEN;
				/* Add comments*/
			}
		}
		if(s=='*')
		{
			loop()
			{
				in++;
				s = DSBYTE[in];
				if(s=='*')if(DSBYTE[in+1]=='/')
				{
					in+=2;
					goto NEXT_TOKEN;
				}
			}
		}
	}
	
	if (strchr("=<>!~&|#",s))
	{
		loop()
		{
			if (!strchr("=<>!~&|#",s)) break;
			
			DSBYTE[pos] = s;
			pos++;
			
			in++;
			s = DSBYTE[in];
		}
		type = LEX_IND;
	}
	else if (strchr(";(,)}{[]+-.*/:^%?$@№`",s))
	{
		DSBYTE[pos] = s;
		pos++;
		type = LEX_IND;
		in++;
	}
	else if(s>='0')&&(s<='9')
	{
		loop()
		{
			if(s<'0')||(s>'9')if(s!='.')break;
			
			DSBYTE[pos] = s;
			pos++;
			
			in++;
			s = DSBYTE[in];
		}
		type = LEX_DEC;
	}
	else if(s>='A')&&(s<='z')&&(!strchr("[]\\^`",s))
	{
		loop()
		{
			if(s<'A')||(s>'z')if(s<'0')||(s>'9')break;
			if(strchr("[]\\^`",s))break;
			
			DSBYTE[pos] = s;
			pos++;
			
			in++;
			s = DSBYTE[in];
		}
		
		loop()
		{
			s = DSBYTE[in];
			if(s!=9)if(s!=10)if(s!=13)if(s!=32)break;
			in++;
			text++;
		}
		type = LEX_VAR;
		if(s=='(')type = LEX_FNC;
	}
	else if(s=='"')||(s=='\'')
	{
		quote = s;
		in++;
		s = DSBYTE[in];
		loop()
		{
			if(s=='\\')
			{
				in++;
				s = DSBYTE[in];
				if(!s){type = LEX_STR;goto GOTO_LEX_END;}
				if(!cmd)switch(s)
				{
					case 'n':s='\n';break;
					case 'r':s='\r';break;
					case 't':s='\t';break;
				}
				else {
					DSBYTE[pos] = '\\';
					pos++;
				}
				goto LEX_STEP_1;
			}
			if(!s){type = LEX_STR;goto GOTO_LEX_END;}
			else if(s==quote)break;
			LEX_STEP_1:
			DSBYTE[pos] = s;
			pos++;
			in++;
			s = DSBYTE[in];
		}
		in++;
		type = LEX_STR;
	}
	else {
		in++;
		type = LEX_NUL;
		DSBYTE[pos] = s;
		pos++;
	}
	GOTO_LEX_END:
	length = in-text;
	text = in;
	DSBYTE[pos] = 0;
	token = #const_token_lexer;
	return token;
}

#endif