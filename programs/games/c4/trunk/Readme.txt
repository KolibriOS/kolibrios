C4 0.1
Copyright (C) 2002 Thomas Mathys
killer@vantage.ch


What's this ?
-------------

C4 is a connect four clone for MenuetOS
(http://www.menuetos.org).


Installation
------------

Simply copy the executable (C4) onto your MenuetOS floppy.
You don't need to copy anything else, the image files are
linked into the executable.

If you want to compile C4 yourself you need NASM.
Get it from http://nasm.sourceforge.net.

Compile C4 using the following command:

	nasm -f bin -o c4 c4.asm
	
(nasmw if you have the win32 version)
