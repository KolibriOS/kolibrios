#define buffer 1024

:struct VAR
{
        dword   data;
        byte   type;
        dword   length;
		dword stack;
};


:void load_const(dword data,type,length)
{
	dword m = 0;
	dword v = 0;
	dword i = 0;
	dword b = 0;
	dword o = 0;
	dword t = 0;
	v = malloc(30);
	switch(type)
	{
		case PY_STR:
			b = length+MEMBUF;
			m = malloc(b);
			t = m;
			i = 0;
			while(i<length) 
			{
				DSBYTE[t] = DSBYTE[data];
				t++;
				data++;
				i++;
			}
			DSDWORD[v] = m;
			DSDWORD[v+5] = length;
			DSDWORD[v+9] = b;
		break;
		case PY_INT:
			DSDWORD[v] = data;
		break;
		case PY_BOOL:
		case PY_FNC:
			DSDWORD[v] = data;
		break;
		default:
			DSDWORD[v] = 0;
		break;
	}
	DSBYTE[v+4] = type;
	
	DSDWORD[v+13] = HASH;
	HASH++;
	
	pushStack(v);
}

:void compare_op(byte cmd)
{
	dword n = 0;
	dword y = popStack();
	dword x = popStack();
	dword dx = 0;
	dword dy = 0;
	dword i = 0;
	
	checkType(x,y);
	
	n = malloc(30);
	
	DSDWORD[n+13] = HASH;
	HASH++;
	
	DSDWORD[n+4] = PY_BOOL;
	
	IF(DSBYTE[x+4] == PY_STR)
	{
		IF(DSDWORD[x+5] == DSDWORD[y+5])
		{
			dx = DSDWORD[x];
			dy = DSDWORD[y];
			
			i = DSDWORD[x+5];
			
			WHILE(i)
			{
				IF(DSBYTE[dx]!=DSBYTE[dy]) BREAK;
				dx++;
				dy++;
				i--;
			}
			IF(!i) DSDWORD[n] = 1;
		}
		pushStack(n);
		return;
	}
	
	SWITCH(cmd)
	{
		CASE 0:
			IF(DSDWORD[x] == DSDWORD[y]) DSDWORD[n] = 1;
		BREAK;
		CASE 1:
			IF(DSDWORD[x] != DSDWORD[y]) DSDWORD[n] = 1;
		BREAK;
		CASE 2:
			IF(DSDWORD[x] >= DSDWORD[y]) DSDWORD[n] = 1;
		BREAK;
		CASE 3:
			IF(DSDWORD[x] <= DSDWORD[y]) DSDWORD[n] = 1;
		BREAK;
		CASE 4:
			IF(DSDWORD[x] > DSDWORD[y]) DSDWORD[n] = 1;
		BREAK;
		CASE 5:
			IF(DSDWORD[x] < DSDWORD[y]) DSDWORD[n] = 1;
		BREAK;
	}
	pushStack(n);
}
:byte pop_jump_if(byte cmd)
{
	dword x = 0;

	x = popStack();
	//test(DSDWORD[x+4],1);
	SWITCH(DSBYTE[x+4])
	{
		CASE PY_INT:
		CASE PY_BOOL:
			IF(cmd){ IF (DSDWORD[x]) RETURN 1;}
			IF(!cmd){ IF (!DSDWORD[x]) RETURN 1;}
		BREAK;
		CASE PY_STR:
			IF(cmd){ IF (DSDWORD[x+5]) RETURN 1;}
			IF(!cmd){ IF (!DSDWORD[x+5]) RETURN 1;}
		BREAK;
	}
	RETURN 0;
}
:void pop_block()
{
	stackWhile-=4;
}
:void binary(byte cmd)
{
	dword x = 0;
	dword y = 0;
	dword l1 = 0;
	dword l2 = 0;
	dword i = 0;
	dword n = 0;
	byte type = 0;
	dword buffer = 0;
	dword length = 0;
	dword position = 0;
	
	y = popStack();
	x = popStack();
	
	IF(cmd==PY_ADD) checkType(x,y);
	
	n = malloc(30);
	
	type = DSBYTE[x+4];
	DSBYTE[n+4] = type;
	
	if(type==PY_STR)
	{
		
		l1 = DSDWORD[x+5];
		
		IF(cmd==PY_ADD) l2 = DSDWORD[y+5];
		ELSE
		{
			i = DSDWORD[y];
			l2 = i*l1;
		}
		
		length = l1+l2;
		
		buffer = length+MEMBUF;
		
		position = malloc(buffer);
		DSDWORD[n] = position;
		
		DSDWORD[n+5] = length;
		DSDWORD[n+9] = buffer;
		
		IF(cmd==PY_ADD) 
		{
			strlcpy(position,DSDWORD[x],l1);
			strlcpy(position+l1,DSDWORD[y],l2);
		}
		ELSE IF(cmd==PY_POW)
		{
			WHILE(i)
			{
				strlcpy(position,DSDWORD[x],l1);
				position+=l1;
				i--;
			}
		}
		goto RETURN_BINARY;
	}
	if(type==PY_INT)
	{
		IF(cmd==PY_ADD)
		{
			DSDWORD[n] = DSDWORD[x] + DSDWORD[y];
			goto RETURN_BINARY;
		}
		IF(cmd==PY_POW) 
		{
			length = DSDWORD[y];
			position = DSDWORD[x];
			DSDWORD[n] = 1;
			WHILE(length) {DSDWORD[n]*=position; length--;}
			goto RETURN_BINARY;
		}
		IF(cmd==PY_SUB) 
		{
			DSDWORD[n] = DSDWORD[x] - DSDWORD[y];
			goto RETURN_BINARY;
		}
		IF(cmd==PY_MUL)
		{
			DSDWORD[n] = DSDWORD[x] * DSDWORD[y];
			goto RETURN_BINARY;
		}
		IF(cmd==PY_MOD)
		{
			DSDWORD[n] = DSDWORD[x] % DSDWORD[y];
			goto RETURN_BINARY;
		}
		IF(cmd==PY_XOR)
		{
			DSDWORD[n] = DSDWORD[x] ^ DSDWORD[y];
			goto RETURN_BINARY;
		}
		IF(cmd==PY_AND)
		{
			DSDWORD[n] = DSDWORD[x] & DSDWORD[y];
			goto RETURN_BINARY;
		}
		IF(cmd==PY__OR)
		{
			DSDWORD[n] = DSDWORD[x] | DSDWORD[y];
			goto RETURN_BINARY;
		}
		IF(cmd==PY_LSH)
		{
			DSDWORD[n] = DSDWORD[x] << DSDWORD[y];
			goto RETURN_BINARY;
		}
		IF(cmd==PY_RSH)
		{
			DSDWORD[n] = DSDWORD[x] >> DSDWORD[y];
			goto RETURN_BINARY;
		}
	}
	RETURN_BINARY:
	pushStack(n);
}

:byte del(dword x)
{
	IF(x)
	{
		IF(DSDWORD[x+9]) free(DSDWORD[x]);
		free(x);
		
		RETURN 1;
	}
	RETURN 0;
}
:void make_function()
{
	stack-=4;
}
:dword store_name(dword x)
{
	dword z = 0;
	dword y = 0;
	y = popStack();
	z = DSDWORD[x];
	IF(z) IF(y != z) del(z);
	DSDWORD[x] = y;
	RETURN y;
}

:void load_name(dword addr)
{
	pushStack(addr);
}
:void pop_top()
{
	stack-=4;
}
:void load_fast(dword addr)
{
	popFast();
	DSDWORD[addr] = EAX;
	pushStack(EAX);
}
:void call_function(dword arg)
{
	dword x = 0;

	dword c = arg;
	dword o = 0;
	
	dword v = 0;
	
	WHILE(c) 
	{
		popStack();
		pushFast(EAX);
		c--;
	}

	v = popStack();
	
	IF(DSBYTE[v+4]!=PY_FNC) test("No function!",0);
	
	v = DSDWORD[v];
	IF(!v)
	{
		test(1,1);
		pushStack(0);
		return;
	}
	v(arg);
	pushStack(EAX);
}

/*

:void call_function(dword arg)
{
	dword x = 0;

	dword c = arg;
	dword o = 0;
	
	dword v = 0;
	
	WHILE(c) 
	{
		o+=4;
		popStack();
		$push eax
		c--;
	}
	$push arg
	
	v = popStack();
	
	IF(DSBYTE[v+4]!=PY_FNC) test("No function!",0);
	
	v = DSDWORD[v];
	IF(!v)
	{
		test(1,1);
		pushStack(0);
		return;
	}
	
	$call v
	ESP+=o;
	
	pushStack(EAX);
}
*/