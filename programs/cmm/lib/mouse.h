#ifndef INCLUDE_MOUSE_H
#define INCLUDE_MOUSE_H

//Button MOUSE
#define MOUSE_LEFT   001b
#define MOUSE_RIGHT  010b
#define MOUSE_LR     011b
#define MOUSE_MIDDLE 100b

/**
 *  The structure of the MOUSE
 *  x - coordinate X
 *  y - coordinate Y
 *  xx and yy - time coordinates
 *  lkm - left MOUSE button
 *  pkm - right MOUSE button
 *  mkm - MOUSE wheel
 *  key - keycode button
 *  tmp - time keycode 
 *  down - key event press
 *  up - key release events
 *  move - event MOUSE movements
 *  click - when clicked
 *  dblclick - double-click get system value
 *  drag - drag the element event
 */

:struct MOUSE
{
	signed x,y,xx,yy,lkm,mkm,pkm,key,tmp,tmp_time,hor,vert,left,top;
	bool down,up,move,click,dblclick,drag;
	dword handle,_;
	byte cmd;
	void get();
	void set();
	void center();
	dword hide();
	void slider();
	void show();
} mouse;

:void MOUSE::show()
{
	if(!handle)return;
	ECX = handle;
	EAX = 37;
	EBX = 5;
	$int 64;
}
:dword MOUSE::hide()
{
	if(!_)
	{
		EAX = 68;
		EBX = 12;
		ECX = 32*32*4;
		$int 64
		ECX = EAX;
		_ = EAX;
	} else ECX = _;
	EAX = 37;
	EBX = 4;
	DX  = 2;
	$int 64;
	handle = EAX;
	ECX = EAX;
	EAX = 37;
	EBX = 5;
	$int 64;
	handle = EAX;
}

//set new attributes MOUSE
:void MOUSE::set()
{
	if((xx!=x)||(yy!=y))
	{
		EAX = 18;
		EBX = 19;
		ECX = 4;
		EDX = (x<<16)+y;
		$int 64
		//move = true;
	}
	if((key)||(lkm|mkm|pkm))&&(down|up|click|dblclick|move)
	{
		if(lkm|mkm|pkm)key=(lkm)|(pkm<<1)|(2<<mkm);
		EAX = 18;
		EBX = 19;
		ECX = key;
		EDX = (x<<16)+y;
		$int 64
	}
}

:void MOUSE::center()
{
	EAX = 18;
	EBX = 15;
	$int 64
}

//get new attributes MOUSE
:void MOUSE::get()
{
	EAX = 37;
	EBX = 1;
	$int	64
	$mov	ebx, eax
	$shr	eax, 16
	$and	ebx,0x0000FFFF
	x = EAX;
	y = EBX;
	if (x>6000) x-=65535;
	if (y>6000) y-=65535;
	EAX = 37;
	EBX = 2;
	$int	64
	$mov	ebx, eax
	$mov	ecx, eax
	key = EAX;
	$and	eax, 0x00000001
	$shr	ebx, 1
	$and	ebx, 0x00000001
	$shr	ecx, 2
	$and	ecx, 0x00000001
	lkm = EAX;
	pkm = EBX;
	mkm = ECX;
	
	//when you release the MOUSE button
	// Mouse Move Event
	if(xx!=x)||(yy!=y)
	{
		move = true;
		xx = x;
		yy = y;
	}
	else move = false;
	// Mouse Up Event
	if(cmd)&&(!key){
		up   = true;
		down = false;
		drag = false;
		if(!move) click = true;
		if(GetStartTime()-GetMouseDoubleClickDelay()<=tmp_time)
		{ 
			dblclick = true;
			click    = false; 
		}
		tmp_time = GetStartTime();
		//returns the key code
		key = tmp;
		lkm = 1&tmp;
		pkm = 2&tmp;
		pkm >>= 1;
		mkm = 4&tmp;
		mkm >>= 2;
		cmd = false;
	}
	
	//when you press the MOUSE button
	// Mouse Down Event/Move Event
	else 
	{
	    up       = false;
		click    = false;
		dblclick = false;
		down     = false;
		// Mouse Move Event
		if(key)if(!cmd) 
		{
			down = true;
			if(move)drag = true;
			cmd = true;
			tmp=key;
		}
	}
	
	//scroll
	EAX = 37;
	EBX = 7;
	$int	64
	$mov	ebx, eax
	$shr	eax, 16
	$and	ebx,0x0000FFFF
	hor = EAX;
	vert = EBX;
}

:void MOUSE::slider()
{
	signed _x,_y;
	if(!handle)hide();
	get();
	_x = x;_y = y;
	pause(5);
	get();
	left = _x - x;
	top  = _y - y;
	center();
	get();
	_x = x;_y = y;
	pause(5);
}


/*=====================================================================================
===========================                                 ===========================
===========================              SYSTEM             ===========================
===========================                                 ===========================
=====================================================================================*/


inline fastcall int GetMouseSpeed() {
	EAX = 18;
	EBX = 19;
	ECX = 0;
	$int 64
}

inline fastcall void SetMouseSpeed(EDX) {
	EAX = 18;
	EBX = 19;
	ECX = 1;
	$int 64
}

inline fastcall int GetMouseAcceleration() {
	EAX = 18;
	EBX = 19;
	ECX = 2;
	$int 64
}

inline fastcall void SetMouseAcceleration(EDX) {
	EAX = 18;
	EBX = 19;
	ECX = 3;
	$int 64
}

inline fastcall int GetMouseDoubleClickDelay() {
	EAX = 18;
	EBX = 19;
	ECX = 6;
	$int 64
}

inline fastcall void SetMouseDoubleClickDelay(DL) {
	EAX = 18;
	EBX = 19;
	ECX = 7;
	$int 64
}

#endif