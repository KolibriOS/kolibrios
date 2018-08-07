#define buffer 1024

:struct VAR
{
        dword   data;
        byte   type;
        dword   length;
		dword stack;
};


:dword build_map(dword count)
{
	dword o = 0;
	dword a1 = 0;
	dword a2 = 0;
	IF(count>8) o = arrayInit(count);
	ELSE o = arrayInit(8);
	WHILE(count)
	{
		a1 = popStack();
		a2 = popStack();
		SWITCH(DSBYTE[a2+4])
		{
			CASE PY_STR:
				dictSet(#o,DSDWORD[a2],a1);
			BREAK;
			CASE PY_INT:
				arraySet(#o,1,a1);
			BREAK;
		}
		count--;
	}
	load_const(o,PY_DCT,0);
}

:void load_const(dword data,type,length)
{
	DSDWORD[#RDATA] = data;
	DSBYTE[#RDATA+4] = type;
	DSDWORD[#RDATA+5] = length;
	DSDWORD[#RDATA+9] = 0;
	
	pushStack(#RDATA);
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
			IF(!i) load_const(1,PY_BOOL,0);
			ELSE load_const(0,PY_BOOL,0);
		}
		return;
	}
	
	IF(!cmd)IF(DSDWORD[x] == DSDWORD[y])
	{
		load_const(1,PY_BOOL,0);
		return;
	}
	IF(cmd==1)IF(DSDWORD[x] != DSDWORD[y])
	{
		load_const(1,PY_BOOL,0);
		return;
	}
	IF(cmd==2)IF(DSDWORD[x] >= DSDWORD[y])
	{
		load_const(1,PY_BOOL,0);
		return;
	}
	IF(cmd==3)IF(DSDWORD[x] <= DSDWORD[y]) 
	{
		load_const(1,PY_BOOL,0);
		return;
	}
	IF(cmd==4)IF(DSDWORD[x] > DSDWORD[y])
	{
		load_const(1,PY_BOOL,0);
		return;
	}
	IF(cmd==5)IF(DSDWORD[x] < DSDWORD[y])
	{
		load_const(1,PY_BOOL,0);
		return;
	}
	load_const(0,PY_BOOL,0);
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
	stackLoop-=4;
}
:void unary_invert()
{
	dword x = 0;
	x = DSDWORD[stack-4];
	EAX = DSDWORD[x];
	$not eax;
	DSDWORD[x] = EAX;
}
:void inplace(byte cmd)
{
	dword buffer = 0;
	dword length = 0;
	
	Y = popStack();
	X = DSDWORD[stack-4];
	
	IF(cmd==PY_ADD) checkType(X,Y);
	
	E = DSBYTE[X+4];
	
	if(E==PY_STR)
	{
		
		A = DSDWORD[X+5];
		
		IF(cmd==PY_ADD) B = DSDWORD[Y+5];
		ELSE
		{
			C = DSDWORD[Y];
			B = C*A;
		}
		
		length = A+B;
		
		buffer = length+MEMBUF;
		
		D = malloc(buffer);

		IF(cmd==PY_ADD) 
		{
			
			memcpy(D,DSDWORD[X],A);
			memcpy(D+A,DSDWORD[Y],B);
		}
		ELSE IF(cmd==PY_MUL)
		{
			B = D;
			E = DSDWORD[X];
			WHILE(C)
			{
				memcpy(B,E,A);
				B+=4;
				C--;
			}
		}

		DSDWORD[#RDATA] = D;
		DSBYTE[#RDATA+4] = PY_STR;
		DSDWORD[#RDATA+5] = length;
		DSDWORD[#RDATA+9] = buffer;
		
		pushStack(#RDATA);
		
		return;
	}
	if(E==PY_INT)
	{
		switch(cmd)
		{
			case PY_ADD:
				DSDWORD[X] += DSDWORD[Y];
			break;
			case PY_POW:
				length = DSDWORD[Y];
				D = DSDWORD[X];
				DSDWORD[X] = 1;
				WHILE(length) {DSDWORD[X]*=D; length--;}
			break;
			case PY_SUB:
				DSDWORD[X] -= DSDWORD[Y];
			break;
			case PY_MUL:
				DSDWORD[X] *= DSDWORD[Y];
			break;
			case PY_MOD:
				DSDWORD[X] = DSDWORD[X]%DSDWORD[Y];
			break;
			case PY_XOR:
				DSDWORD[X] ^= DSDWORD[Y];
			break;
			case PY_AND:
				DSDWORD[X] &= DSDWORD[Y];
			break;
			case PY__OR:
				DSDWORD[X] |= DSDWORD[Y];
			break;
			case PY_LSH:
				DSDWORD[X] <<= DSDWORD[Y];
			break;
			case PY_RSH:
				DSDWORD[X] >>= DSDWORD[Y];
			break;
			case PY_FDV:
				DSDWORD[X] /= DSDWORD[Y];
			break;
			case PY_TDV:
				DSDWORD[X] /= DSDWORD[Y];
			break;
		}
	}
}
:void binary(byte cmd)
{
	dword buffer = 0;
	dword length = 0;
	
	Y = popStack();
	X = popStack();
	
	IF(cmd==PY_ADD) checkType(X,Y);
	
	E = DSBYTE[X+4];
	
	if(E==PY_STR)
	{
		
		A = DSDWORD[X+5];
		
		IF(cmd==PY_ADD) B = DSDWORD[Y+5];
		ELSE
		{
			C = DSDWORD[Y];
			B = C*A;
		}
		
		length = A+B;
		
		buffer = length+MEMBUF;
		
		D = malloc(buffer);

		IF(cmd==PY_ADD) 
		{
			
			memcpy(D,DSDWORD[X],A);
			memcpy(D+A,DSDWORD[Y],B);
		}
		ELSE IF(cmd==PY_MUL)
		{
			B = D;
			E = DSDWORD[X];
			WHILE(C)
			{
				memcpy(B,E,A);
				B+=4;
				C--;
			}
		}

		DSDWORD[#RDATA] = D;
		DSBYTE[#RDATA+4] = PY_STR;
		DSDWORD[#RDATA+5] = length;
		DSDWORD[#RDATA+9] = buffer;
		
		pushStack(#RDATA);
		
		return;
	}
	if(E==PY_INT)
	{
		switch(cmd)
		{
			case PY_ADD:
				A = DSDWORD[X] + DSDWORD[Y];
			break;
			case PY_POW:
				length = DSDWORD[Y];
				D = DSDWORD[X];
				DSDWORD[Z] = 1;
				WHILE(length) {A*=D; length--;}
			break;
			case PY_SUB:
				A = DSDWORD[X] - DSDWORD[Y];
			break;
			case PY_MUL:
				A = DSDWORD[X] * DSDWORD[Y];
			break;
			case PY_MOD:
				A = DSDWORD[X] % DSDWORD[Y];
			break;
			case PY_XOR:
				A = DSDWORD[X] ^ DSDWORD[Y];
			break;
			case PY_AND:
				A = DSDWORD[X] & DSDWORD[Y];
			break;
			case PY__OR:
				A = DSDWORD[X] | DSDWORD[Y];
			break;
			case PY_LSH:
				A = DSDWORD[X] << DSDWORD[Y];
			break;
			case PY_RSH:
				A = DSDWORD[X] >> DSDWORD[Y];
			break;
			case PY_FDV:
				A = DSDWORD[X] / DSDWORD[Y];
			break;
			case PY_TDV:
				A = DSDWORD[X] / DSDWORD[Y];
			break;
		}
		load_const(A,PY_INT,0);
	}
}

:byte del(dword v)
{
	dword x = 0;
	x = DSDWORD[v];
	IF(x)
	{
		IF(DSDWORD[x+9]) free(DSDWORD[x]);
		free(x);
		DSDWORD[v] = 0;
		RETURN 1;
	}
	RETURN 0;
}
:void build_class(dword count)
{
	dword name = 0;
	dword func = 0;
	func = popFast();
	name = popFast();
	load_const(func,PY_CLS,0);
}
:void load_build_class()
{
	load_const(#build_class,PY_FNC,0);
}
:void make_function()
{
	stack-=4;
}
:void store_name(dword x)
{
	dword tmp = 0;
	dword stk = 0;
	stk = popStack();
	IF(!DSDWORD[x])
	{
		tmp = malloc(30);
		memcpy(tmp,stk,30);
		DSDWORD[x] = tmp;
		return;
	}
	memcpy(DSDWORD[x],stk,30);
}
:void rot(dword x,y)
{
	EAX = DSDWORD[stack-8];
	DSDWORD[stack-8] = DSDWORD[stack-4];
	DSDWORD[stack-4] = EAX;
}
:void build_tuple(dword len)
{
	dword tuple = 0;
	dword buf = 0;
	dword l = 0;
	l = len;
	tuple = malloc(4*len+MEMBUF);
	buf = tuple;
	WHILE(len)
	{
		DSDWORD[buf] = popStack();
		buf+=4;
		len--;
	}
	load_const(tuple,PY_TPL,l);
}
:void unpack_sequence(dword len)
{
	dword tuple = 0;
	dword x = 0;
	popStack();
	tuple = DSDWORD[EAX];

	WHILE(len)
	{
		pushStack(DSDWORD[tuple]);
		tuple+=4;
		len--;
	}
}
:void rot_two()
{
	rot(DSDWORD[stack-8],DSDWORD[stack-4]);
}
:void rot_three()
{
	rot(DSDWORD[stack-12],DSDWORD[stack-4]);
	rot(DSDWORD[stack-8],DSDWORD[stack-4]);
}
:void load_name(dword addr)
{
	pushStack(addr);
}
:void load_global(dword addr)
{
	pushStack(addr);
}
:dword strlen(dword txt)
{
	dword i = 0;
	i = txt;
	WHILE(DSBYTE[i]) i++;
	RETURN i - txt;
}
:dword hashString(dword x)
{
	dword l = 0;
	dword h = 0;
	l = x;
	WHILE(DSBYTE[x])
	{
		h+=DSBYTE[x];
		h<<=1;
		x++;
		h/=x-l;
	}
	return h;
}


:void pop_top()
{
	stack-=4;
}
:void load_fast(dword addr)
{
	dword x = 0;
	IF(!DSDWORD[addr]) 
	{
		x = popFast();
		DSDWORD[addr] = x;
	}
	ELSE x = DSDWORD[addr];
	pushStack(x);
}
:void call_function(dword arg)
{
	dword count = 0;
	dword func = 0;
	byte type = 0;
	dword stackRet = 0;
	count = arg;
	
	WHILE(count) 
	{
		pushFast(popStack());
		count--;
	}
	
	func = popStack();
	
	//IF(DSBYTE[Z+4]!=PY_FNC) test("No function!",0);
	type = DSBYTE[func+4];
	
	func = DSDWORD[func];
	IF(!func)
	{
		test1("Null function!",0);
		pushStack(0);
		return;
	}
	IF(type==PY_FNC)
	{
		stackRet = stack;
		func(arg);
		IF(stackRet == stack) load_const(0,PY_NONE,0);
	}
	ELSE IF(type==PY_CLS)
	{
		stackRet = stack;
		func(arg);
		IF(stackRet == stack) load_const(0,PY_NONE,0);
	}
	stackFast = beginStackFast;
}
:void call_method(dword arg)
{
	dword count = 0;
	dword func = 0;
	byte type = 0;
	dword stackRet = 0;
	count = arg;
	
	WHILE(count) 
	{
		pushFast(popStack());
		count--;
	}
	
	func = popStack();
	
	//IF(DSBYTE[Z+4]!=PY_FNC) test("No function!",0);
	type = DSBYTE[func+4];
	
	func = DSDWORD[func];
	IF(!func)
	{
		test1("Null function!",0);
		pushStack(0);
		return;
	}
	IF(type==PY_FNC)
	{
		stackRet = stack;
		func(arg);
		IF(stackRet == stack) load_const(0,PY_NONE,0);
	}
	ELSE IF(type==PY_CLS)
	{
		func(arg);
		pushStack(EAX);
	}
	stackFast = beginStackFast;
}
:void load_method(dword name)
{
	dword x = 0;
	dword data = 0;
	dword get = 0;
	x = popStack();
	data = DSDWORD[x];
	get = dictGet(DSDWORD[data],name);
	pushStack(get);
}
:void append(dword count)
{
	dword x = 0;
	dword y = 0;
	x = popFast();
	y = popFast();
	test1(DSBYTE[x+4],1);
}
:void build_list(dword count)
{
	dword n = 0;
	dword mem = 0;
	dword i = 0;
	dword tmp = 0;
	dword method = 0;

	mem = malloc(count*4+MEMBUF);
	i = count;
	n = i*4+mem+4;
	
	load_const(#append,PY_FNC,0);
	dictSet(#method,"append",popStack());
	
	DSDWORD[mem] = method;
	WHILE(i)
	{
		n-=4;
		tmp = malloc(30);
		memcpy(tmp,popStack(),30);
		DSDWORD[n] = tmp;
		i--;
	}
	load_const(mem,PY_LST,count);
}
:dword store_fast()
{
	return popStack();
}

:void binary_subscr()
{
	dword x = 0;
	dword y = 0;
	dword l1 = 0;
	dword l2 = 0;
	dword tmp = 0;
	dword ret = 0;
	byte t = 0;
	
	x = popStack();
	y = popStack();
	
	//IF(DSBYTE[x+4]!=PY_INT) error type;

	l1 = DSDWORD[y+5];
	
	t = DSBYTE[y+4];
	IF(t==PY_LST)
	{
		IF(l1<=x) test1("error max list",0);
		IF(0>l1) test1("error min list",0);
		
		y = DSDWORD[y];
		pushStack(DSDWORD[x*4+y]);
		return;
	}
	IF(t==PY_DCT)
	{
		y = DSDWORD[y]; // data object
		SWITCH(DSBYTE[x+4])
		{
			CASE PY_INT:
				
				test1(DSDWORD[y],1);
				x = arrayGet(DSDWORD[y],DSDWORD[x]);
			BREAK;
			CASE PY_STR:
				x = dictGet(DSDWORD[y],DSDWORD[x]);
			BREAK;
		}
		pushStack(x);
		//test1(DSBYTE[x+4],1);
		return;
	}
	IF(t==PY_STR)
	{
		x = DSDWORD[x];
		IF (l1<=x)||(l1<0) test1("IndexError: string index out of range",0);
		y = DSDWORD[y];
		tmp = malloc(MEMBUF);
		ret = malloc(30);
		DSBYTE[tmp] = DSBYTE[x+y];
		DSDWORD[ret] = tmp;
		DSBYTE[ret+4] = PY_STR;
		DSDWORD[ret+5] = 1;
		DSDWORD[ret+9] = MEMBUF;
		pushStack(ret);

		return;
	}
}
:void build_slice(dword count)
{
	
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