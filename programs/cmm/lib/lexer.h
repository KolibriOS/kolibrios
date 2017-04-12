#ifndef INCLUDE_LEXER_H
#define INCLUDE_LEXER_H

#ifndef INCLUDE_STRING_H
#include "../lib/strings.h"
#endif

#ifndef INCLUDE_MEM_H
#include "../lib/mem.h"
#endif
/** Splits text into tokens
 *  Author  :Pavel Yakovlev
 *  Homepage:https://vk.com/pavelyakov39
 *  Ver.    : 1.51
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
	dword token,text,mem_list,count,buffer_loading;
	dword str_buffer;
	byte type;
	char quote;
	signed length;
	dword next(void);
	dword back(void);
	dword list(void);
	void free(void);
	dword position(dword __);
	void load(dword _text);
	void expected(dword _text);
};
:dword back(void)
{
	
}
:dword lexer::list(void)
{
	dword count_mem,buf_loop,pos;
	count_mem = 0;
	buf_loop  = 5000; // на тыс элементов.
	count = 0;
	buffer_loading = malloc(buf_loop);
	pos = buffer_loading;
	while(type!=LEX_END)
	{
		pos+=count_mem;
		next();
		DSDWORD[pos] = strndup(token,length);
		pos+=4;
		DSBYTE [pos] = type;
		pos++;
		count++;
		if(pos-buffer_loading>buf_loop)
		{
			buf_loop*=2;
			buffer_loading = realloc(buffer_loading,buf_loop);
		}
	}
	return buffer_loading;
}
:void lexer::free(void)
{
	dword z;
	z = count;
	while(z)
	{
		z--;
		position(z);
		::free(token);
	}
	count = 0;
	::free(buffer_loading);
}
:dword lexer::position(dword __)
{
	dword pos1;
	if(!count)list();
	if(__>=count)__=count-1;
	else if(__<0)__=0;
	pos1 = __*5;
	pos1 += buffer_loading;
	token = DSDWORD[pos1];
	pos1++;
	type = DSBYTE[pos1];
	return token;
}
:void lexer::expected(dword _text)
{
	notify(_text);
	ExitProcess();
}

:void lexer::load(dword _text)
{
	text = _text;
	count = 0;
	str_buffer = 0;
}

:dword lexer::next(void)
{
	char s;
	dword len_str_buf,tmp;
	dword pos,in;
	pos = #const_token_lexer;
	in = text;
	//len_str_buf = 1024;
	if(str_buffer)::free(str_buffer);
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
		tmp = in;
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
			/*if(in-tmp>len_str_buf)
			{
				if(str_buffer)
				{
					tmp = len_str_buf;
					len_str_buf+=1024;
					str_buffer = realloc(str_buffer,len_str_buf+1);
					strlcpy(str_buffer+tmp,#const_token_lexer,1024);
					pos = #const_token_lexer;
				}
				else {
					len_str_buf+=1024;
					str_buffer = malloc(len_str_buf+1);
					strlcpy(str_buffer,#const_token_lexer,1024);
					pos = #const_token_lexer;
				}
			}*/
			s = DSBYTE[in];
		}
		in++;
		/*tmp = pos-in;
		if(str_buffer)
		{
			if(tmp)
			{
				str_buffer = realloc(str_buffer,tmp+1);
				strlcpy(str_buffer+len_str_buf,#const_token_lexer,tmp);
			}
			type = LEX_STR;
			length = len_str_buf+tmp;
			text = in;
			tmp = str_buffer+length;
			DSBYTE[tmp] = 0;
			token = str_buffer;
			return token;
		}*/
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