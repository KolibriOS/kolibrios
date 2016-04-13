README
======

General and licensing information
---------------------------------
AVRA v1.3.0 - Assember for the Atmel AVR microcontroller family

Licensing information
~~~~~~~~~~~~~~~~~~~~~

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version. Please read below for for information.

Disclaimer
~~~~~~~~~~

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

GNU General Public License
~~~~~~~~~~~~~~~~~~~~~~~~~~

You should have received a copy of the GNU General Public License
along with this program; see the file "COPYING".  If not, visit
http://www.gnu.org or write to the Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA 02111-1307, USA. You can
also contact the authors of AVRA to receive a copy of the COPYING file.

Trademarks and copyright
~~~~~~~~~~~~~~~~~~~~~~~~

Atmel, AVR, AVR Studio, Intel, Windows are registered enterprises, brands
and registered trademarks. The mentioned companies have no relation to
AVRA and are therefore not responslible for any problems that occur when
using AVRA. Many thanks for your products, support and efforts.


Introducion
-----------

AVRA is an assembler for Atmel AVR microcontrollers, and it is almost
compatible with Atmel's own assembler AVRASM32. The programming
principles and conceptions are based on the ANSI programming language "C".

The initial version of AVRA was written by John Anders Haugum. He released
all versions until v0.7. All later versions were released by Tobias Weber.


Differences between AVRA and AVRASM32
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are some differences between the original Atmel assembler AVRASM32 and AVRA. Basically AVRA is designed to replace AVRASM32 without special changes in your current Atmel AVR Studio enviroment.
Command line options have been adapted as far as it was possible until now. Jumping to fault containing line directly by double-clicking on the error message in the output window does work as with AVRASM32.

The differences in detail
~~~~~~~~~~~~~~~~~~~~~~~~~

    - Support for some extra preprocessor directives.

      .define, .undef, .ifdef, .ifndef, .if, .else, .endif, .elif, .elseif, .warning

    - Not all command line options are supported. 
      Specifying an eeprom file (-e) is not supported. All eeprom data is
      put out into a file called program.eep.hex and always Intel hex
      format. Other hex file formats than Intel are currently not supported.

    - Forward references not supported for .ifdef and .ifndef directives.
      This makes sure, that directives like .ifdef and .undef are working 
      properly. If you are familiar with the C programming language, you
      should get easily into AVRA. See chapter "Programming techniques" for
      more information about how to write proper code.

    - Enhanced macro support
      AVRA has some new features for writing flexible macros. This should
      increase the ability to reuse code e.g. build your own library.

    - Debugging support
      AVRA creates a coff file everytime the assembly was sucessful. This
      file allows AVR Studio or any coff compatible debugger to simulate
      or emulate the program.

    - Meta tags for assembly time
      This helps you tracking versions of your software and can also be
      used to generate customer specific serial numbers.


Compatibility
~~~~~~~~~~~~~

Since AVRA is written in ANSI C, it should be possible to compile it on
most system platforms. If you have problems compiling AVRA, please leave
a message on the sourceforge message board or send a mail to the
authors of AVRA.


Installation
------------

To install avra you should copy the avra-executable to an apropriate
location. To compile you should rename the appropriate makefile, and
perform a make (use smake for Amiga SAS/C, and nmake for Mickeysoft
visual c++).


Linux
~~~~~

To compile avra you need gcc and the automake utilities. These will create
a ./configure script that evaluates your system enviroment. To get the
AVRA executable, you have to issue the following commands:

aclocal
autoconf
automake -a
./configure
make && make install

AmigaOS
~~~~~~~

avra can be copied any apropriate directory. If you are using the source
distribution a 'make install' will do the same.


Microsoft Windows
~~~~~~~~~~~~~~~~~

If you received the Windows binary package, look into the \bin
directory where you can find avra.exe. This should be copied to any
apropriate location. You can also overwrite AVRASM32.EXE in your
Atmel AVR Studio. If you want to compile it yourself you could download then
OpenWatcom C/C++ Toolchain for windows and create a new project and add the C
and H files to it and compile.

Apple OS X
~~~~~~~~~~

If you recieved the Apple OS X binary package, look into the bin directory this
file is compiled universal and should run on intel 32 and 64 bit and powerpc. If
you want to compile it yourself go to the src directory and invoke `make -f
makefiles/Makefiles.osx` and then the executable should be created.


Synopsis
--------

Command line usage
~~~~~~~~~~~~~~~~~~

    usage: AVRA [-f][O|M|I|G] output file type
             [-o <filename>] output file name
             [-l <filename>] generate list file
             [-m <mapfile>] generate map file
             [--define <symbol>[=<value>]]  [--includedir <dir>] [--listmac]
             [--max_errors <number>] [--devices] [--version]
             [-h] [--help] general help
             [-W NoRegDef] supress register redefinition warnings
             <file to assemble>


Parameter list
~~~~~~~~~~~~~~

    --listfile    -l : Create list file
    --mapfile     -m : Create map file
    --define      -D : Define symbol.
    --includedir  -I : Additional include dirs.
    --listmac        : List macro expansion in listfile.
    --max_errors     : Maximum number of errors before exit
                       (default: 10)
    --devices        : List out supported devices.
    --version        : Version information.
    --help, -h       : This help text.


Warning supression
~~~~~~~~~~~~~~~~~~

Since avra 1.1 there is a possibility to supress certain warnings. 
Currently only register reassignment warnings can be supressed.

Example: avra -W NoRegDef


Programming techniques
----------------------


Using directives
~~~~~~~~~~~~~~~~

AVRA offers a number of directives that are not part of Atmel's
assembler. These directives should help you creating versatile code that
can be designed more modular.

Directive .define
^^^^^^^^^^^^^^^^^

To define a constant, use ".define". This does the same thing as ".equ",
it is just a little more C style. Keep in mind that AVRA is not case
sensitive. Do not mix ".def" and ".define", because ".def" is used to
assign registers only. This is due to backward compatibility to Atmel's
AVRASM32. Here is an example on how .define can be used.

    .define network 1

Now "network" is set to the value 1. You may want to assemble a specific
part of your code depeding on a define or switch setting. You can test
your defined word on existence (.ifdef and .ifndef) as well as on the
value it represents. The following code shows a way to prevent error
messages due to testing undefined constants. Conditional directives must
always end with an .endif directive.

    .ifndef network
    .define network 0
    .endif

Directive .if and .else
^^^^^^^^^^^^^^^^^^^^^^^

The three lines in the last example set the default value of "network".
In the next example, you see how we can use default values. If a constant
has not defined previously, it is set to zero. Now you can test wether
e.g. network support is included into the assemby process.

    .if network = 1
    .include "include\tcpip.asm"
    .else
    .include "include\dummynet.asm"
    .endif

In the second part of the above listing you see the use of .else, which
defines the part of the condition that is being executed if the equation
of the preceding .if statement is not equal. You can also use the else
statement to test another equasion. For that purpose use .elif, which
means "else if". Always close this conditional part with ".endif"

Directive .error
^^^^^^^^^^^^^^^^

This directive can be used to throw errors if a part in the code has reached
that should not be reached. The following example shows how we can stop
the assembly process if a particular value has not been previously set.

    .ifndef network
    .error "network is not configured!" ;the assembler stops here

Directive .nolist and .list
^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ouput to the list file can be paused by this two directives. After
avra discovers a .nolist while assembling, it stops output to the list file.
After a .list directive is detected, it continues the normal list file output.

Directive .includepath
^^^^^^^^^^^^^^^^^^^^^^

By default, any file that is included from within the source file must
either be a single filename or a complete absolute path. With the directive
.includepath you can set an additional include path . Furthermore you can 
set as many include paths as you want. Be sure not no use same filename
in separate includes, because then it is no longer clear which one avra
should take.

Using include files
-------------------

To avoid multiple inclusions of include files, you may use some pre-
processor directives. See example file stack.asm that is being included
into the main programm file as well as in other include files.

    .ifndef _STACK_ASM_
    .define _STACK_ASM_
  
    .include "include/config.inc"

    ; *** stack macro ***
    
    .dseg
    m_stack:    .byte __stack_size__
    .cseg

    .macro      stack_setup
        load    [v:w,m_stack + __stack_size__]
	outp    [SPREG,v:w]
    .endm

    .endif ; avoid multiple inclusion of stack.asm


Using build date meta tags
~~~~~~~~~~~~~~~~~~~~~~~~~~

If you like to implement compiler build time and date into your
program, you can make use of some sepcial tags that avra supports.
    
    %MINUTE%  is being replaced by the current minute (00-59)
    %HOUR%    is being replaced by the current hour (00-23)
    %DAY%     is being replaced by the current day of month (01-31)
    %MONTH%   is being replaced by the current month (01-12)
    %YEAR%    is being replaced by the current year (2004-9999)

    buildtime: .db "Release date %DAY%.%MONTH%.%YEAR% %HOUR%:%MINUTE%"
    
This line will then assembled by avra into:
    
    buildtime: .db "Release date 10.05.2004 19:54"
   
You may also create a self defined serial number with meta tags:
    
    .define serialnumber %DAY% + %MONTH%*31 + (%YEAR% - 2000) *31*12

The %TAG% is translated before any other parsing happens. The real
output can be found in the list file.

Macro features
--------------

Sometimes you have to work with 16 bit or greater variables stored
in 8 bit registers. The enhanced macro support allows you to write short
and flexible macros that simplify access to big variables. The extended
mode is active, as soon as you use parenthesis like this "[ ]" to wrap
macro parameters.

Auto type conversion for macros
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Values representing more than 8 Bits are usualy kept in a set of byte
wide registers. To simplify 16 Bit or greater operations, I added a new
language definitions. Words can be written as r16:r17, whereas register
r16 contains the higher part and register r17 the lower part of this 
16 Bit value.

Macro data types
^^^^^^^^^^^^^^^^

There are 3 data types that can be used. They will be added as character
separated by one underline character.

    immediate values  _i
    registers         _8,_16,_24,_32,_40,_48,_56,_64
    void parameter    _v

16 Bit Source and Destionation registers 'dst' and 'src'

        src = YH:YL
        dst = ZH:ZL

Within the parenthesis, the two words src and dst are interpreted as YH:YL
and ZH:ZL. Normal code outside of the macro parameter parenthesis can
still make use of these special key words "src" and "dst".

Examples for automatic type conversion
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To simplify the parameters in the demonstration below, we need to
redefine some registers.

        .def    a = r16   ; general purpose registers
        .def    b = r17
        .def    c = r18
        .def    d = r19

        .def    w = r20   ; working register
        .def    v = r21   ; working register

If we substract 16 Bit values stored in a, higher byte and b, lower byte
with that in c:d, we usually have to use the following command sequence:

	sub     b,d
	sbc     a,c

Now we can do the following steps to simplify 16 or more Bit manipulations

        .macro  subs
        .message "no parameters specified"
        .endm

        .macro  subs_16_16
        sub     @1,@3
        sbc     @0,@2 
	.endm

        .macro  subs_16_8
	sub     @1,@2
	sbci    @0,0
	.endm

        ;now we can write a 16 Bit subraction as:

        subs    [a:b,c:d]

        ;or for calculating 16 minus 8 Bit

        subs    [a:b,c]


Overloading macros
~~~~~~~~~~~~~~~~~~

Like in you are used to C functions, you can write macros for different
parameter lists. If you would like to have a versatile macro, you can
specify a unique macro for each parameter situation. See the next sample.

        .macro  load

	; this message is shown if you use the macro within your code
	; specifying no parameters. If your macro allows the case where
	; no parameters are given, exchange .message with your code.

	.message "no parameters specified"
        .endm

        ; Here we define the macro "load" for the case it is being used
	; with two registers as first parameter and a immediate (constant)
	; value as second parameter.

        .macro  load_16_i
        ldi     @0,high(@2)
        ldi     @1,low(@2)
        .endm

        ; the same case, but now with a 32 bit register value as first
	; parameter

        .macro  load_32_i
        ldi     @0,BYTE4(@4)
        ldi     @1,BYTE3(@4)
        ldi     @2,high(@4)
        ldi     @3,low(@4)
        .endm

        ; Now let's see how these macros are being used in the code

	load 	[a:b,15]     ;uses macro load_16_i to load immediate

	load	[a:b:c:d,15] ;uses macro load_32_i to load immediate


More examples
~~~~~~~~~~~~~

       .dseg
       counter  .byte 2
       .cseg

       .macro   poke
       .message "no parameters"
       .endm

       .macro   poke_i_16_i
       ldi      @1,high(@3)
       sts      @0+0,@1
       ldi      @2,low(@3)
       sts      @0+1,@2
       .endm

       .macro   poke_i_i
       ldi      w,@1
       sts      @0+0,w
       .endm

       .macro   poke_i_v_i
       ldi      w,high(@3)
       sts      @0+0,w
       ldi      w,low(@3)
       sts      @0+1,w
       .endm

       .macro   poke_i_v_v_v_i
       ldi      w,high(@3)
       sts      @0+0,w
       ldi      w,low(@3)
       sts      @0+1,w
       ldi      w,BYTE3(@3)
       sts      @0+2,w
       ldi      w,BYTE4(@3)
       sts      @0+3,w
       .endm


       ; this writes '9999' into the memory at 'counter'
       ; uses only the working register for transfering the values.

       poke     [counter,w:w,9999]

       ; works same as above, but the transferred value '9999' is also
       ; kept in the pair of register a:b

       poke     [counter,a:b,9999]

       ; in my design 'w' is always working reg. which implies that
       ; it cannot be used for normal variables. The following example
       ; uses poke_i_i because the parameter contains two immediate values.

       poke     [counter,9999] ;uses poke_i_i

       ; to be able to choose between a 8,16 or 32 Bit operation, you just
       ; add a void parameter.

       poke     [counter,,9999] ;uses poke_i_v_i

       ; and the same for 32 Bit pokes

       poke     [counter,,,,9999] ;uses poke_i_v_v_v_i

Loops within macros
~~~~~~~~~~~~~~~~~~~

One problem you may have experienced, is that labels defined within macros
are defined twice if you call the macro for example two times. Now you can
use labels for macro loops. Loops within macros must end with '_%'. the
"%" symbol is replaced by a running number.

Loop example
^^^^^^^^^^^^

       ; Definition of the macro

       .macro   write_8_8
       write_%:
            st      Z+,@0
            dec     @1
            brne    write_%
       .endm

       ; Use in user code

       write   [a,b]
       write   [c,d]

       ; After assembling this code, the result looks like this

       write_1:
            st          Z+,a
            dec         b
            brne        write_1
       write_2:
            st          Z+,c
            dec         d
            brne        write_2

Warnings and Errors
-------------------

Some errors and warnings may confuse you a little bit so we will try to
clear some frequently asked questions about such cases.

Constant out of range
~~~~~~~~~~~~~~~~~~~~~

This warning occurs if a value exceeds the byte or word value of a assignment.
Read the comment posted by Jim Galbraith:

The expression (~0x80) is a Bitwise Not operation.  This 
operator returns the input expression with all its bits 
inverted.  If 0x80 represents -128, then 0x7f, or +127 
should be ok.  If this is considered as a 32-bit expression 
(AVRA internal representation), then it appears to be more 
like oxffffffff-0x80 or 0xffffffff^0x80.  The result would then 
be 0xffffff7f.  The assembler would then have to be told or it 
would have to decide, based on context, how much 
significance to assign to the higher bits.  I have also 
encountered such conditions with various assemblers, 
including AVRA.  To make sure the assembler does what I 
really want, I use a construct like 0xff-0x80 or 0xff^0x80.  
This way the bit significance cannot extend beyond bit-7 and 
there cannot be any misunderstanding.   

Can't use .DB directive in data segment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.DB and .DW is only used to assign constant data in eeprom or code space.
The reason why using it within data segment is forbidden is, that you
cannot set ram content at assembly time. The values must be programmed into
ROM area and at boot read from ROM into RAM. This is up to the user code.
You can only allocate memory for your variables using labels and the .byte
directive.

    .dseg
    my_string: .byte 15

BYTE directive
~~~~~~~~~~~~~~

.BYTE directive can only be used in data segment (.DSEG)

This directive cannot be used in code or eeprom region because this only 
allocates memory without assgning distinct values to it. Please use .db
or .dw instead.

Internal assembler error
~~~~~~~~~~~~~~~~~~~~~~~~

If you get an "Internal assembler error" please contact the project maintainer
by sending him a code example and a description of your working enviroment.


AVRA internals
--------------

This section provides thoughts of the avra internal design. I have to admit
that the code of avra is anything else than clean and optimized. To increase
the code readability I will try to give you some standards that should improve
quality. The following standards are similar to what GNU proposes. 

Coding standards
~~~~~~~~~~~~~~~~

Tab space is always 2 spaces. The Tab character (ascii 9) is not used.
if,while,for are always opened on the same line but closed on the next line.
The closing bracket is in the same column as the first letter of the loop
directive.

    Example:
----
    while(i > 0) { 
     do_something();
    }
----


Credits
-------

We would like to thank the following people for giving contributions,
patches and bug reports, as well as suggestions and new ideas.

----
      Jon Anders Haugum (project founder)
      Burkhard Arenfeld (release 1.2.0)
      Tobias Weber (old maintainer)
      Jerry Jacobs (release 1.3.0)
      Bernt Hembre
      Nils Strøm
      Roberto Biancardi
      Qwerty Jones
      Ben Hitchcock (Maker of the mac port)
      Daniel Drotos
      Laurence Boyd II
      Varuzhan Danielyan
      Laurence Turner
      Eugene R. O'Bryan
      Dmitry Dicky
      Bob Harris (Maker of coff support)
      Tobias Weber (enhanced macro support)
      Lesha Bogdanow
      Jim Galbraith
      Mark Brinicombe
      Igor Nikolayenko
      Peter Hettkamp
      Herb Poppe
      David Burke
      Alexey Pavluchenko
      Alan Probandt
      Mariusz Matuszek
      Arne Rossius
      Marti Tichacek
      Patrick Parity
      Johannes Overmann
      Roland Riegel
      Peter Katzmann
      Donald D. Davis
----

And all the anonymous people who submitted patches!

Thank you for your work and support.

References
----------

    http://www.suprafluid.com/avra
    http://www.avrfreaks.de
    http://www.atmel.com
