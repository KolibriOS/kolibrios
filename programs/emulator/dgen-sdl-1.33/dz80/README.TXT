                         dZ80 Version 2.0 Source Code

                       Copyright 1996-2002 Mark Incley.

                           E-mail: dz80@inkland.org
                            http://www.inkland.org


Serious Bit
-----------

I have made this source code available so that it may be compiled on platforms
other than MS-DOS and Windows. You may compile it and distribute the resulting
executable only if no monies are charged for it.

      ** YOU ARE NOT ALLOWED TO DISTRIBUTE THIS SOFTWARE COMMERICIALLY **


Not So Serious Bit
------------------

If you make any feature modifications to the dZ80 source code, please let me
know, so that I can make them to my source too. I didn't intend for dZ80 to
grow into an all singing and dancin' disassembler, but, if features are added,
I would like to add them to my base version too.


Source Code Notes
-----------------

Please note that as of dZ80 2.0, you will also require the Lua 4.0
source code download from http://www.lua.org. It should be unzipped so that
the lua folder appears within the dZ80 folder as "Lua".


File            Purpose
----            -------
types.h         Sets up the typedefs
tables.c        Miscellaenous tables used by dZ80
dz80.c / h      The "front end" for the disassembler core
dissz80.c / h   The disassembler core
dissz80p.h      Private dZ80 header file - check out EXAMPLE.C
parsecmd.c/h    Command line parser for the front end.
loadfile.c      File loader and memory allocator

example.c       An example of how to drop dZ80 into your own programs to gain
		a disassembler.

makefile        A simple makefile for dZ80
dz80ns.mak      A simple makefile for dZ80 without scripting support
example.mak     Makefile for the example code

skip.c          A self-contained dZ80 opcode map file generator by
                Raffaele Sena


If you have any problems, just e-mail me at dz80@inkland.org

Cheers,

        Mark
