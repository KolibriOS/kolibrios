AClock 1.1
Copyright (c) 2002,2003 Thomas Mathys
killer@vantage.ch


what the hell is this ?
-----------------------

this is aclock, a silly analog clock application
for menuetos (http://www.menuetos.org).


why do i need it ?
------------------

well, this is certainly one of the last programs on
earth you'd ever need. anyway, it demonstrates how
how to do certain things:

- writing menuet apps that parse the command line.
  includes a strtok-like function that you might
  want to use in own projects. or maybe rather not.
- writing menuet apps that are aware of the current
  window size and that have no problems with different
  skin heights.
- how to write menuet apps with nasm instead of fasm
  (there should be a gas version aswell, don't you think ?)
  and how to write kick-ass code with nasm in general =)


compiling instructions
----------------------

yes, it's still written for nasm.
i really can't be bothered to work with fasm.

oh yes, you wanted to know how to compile aclock:

	nasm -t -f bin -o aclock aclock.asm
	
if you get error messages like

	nasm: unrecognised option `-t
	type `nasm -h' for help

then you've got an old version of nasm.
get a newer version (0.98.36 or later) from
http://nasm.sourceforge.net


configuration
-------------

you might want to change some of the constants defined
somewhere at the top of aclock.asm. the following might
be useful:

	-	DEFAULT_XPOS
	-	DEFAULT_YPOS
	-	DEFAULT_WIDTH
	-	DEFAULT_HEIGHT
	-	MIN_WIDTH
	-	MIN_HEIGHT
	
for more info about DEFAULT_XPOS/DEFAULT_YPOS see next
section.


usage
-----

this version of AClock introduces command line parameters.
here's an example command line:

	aclock w128 h128 x20 y-20
	
this creates a window that is 128 pixels wide and 128 pixels
high (that's for the work area, without border/title bar).
the window is placed at x=20, y=screen resolution-20
(because of the minus sign after the y).

all parameters are optional and may appear in any order.
you can't have any whitespaces in a parameter, e.g.
"w 128" is an invalid parameter (which will simply be ignored).
the command line parser is case sensitive.
