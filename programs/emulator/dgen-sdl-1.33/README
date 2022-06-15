--------------------------------------------------------------------------------
WHAT IS THIS?
--------------------------------------------------------------------------------
This is DGen/SDL, a semi-fantastic emulator for Unix-esque operating systems
supported by the Simple DirectMedia Layer (SDL) library. It produces a virtual
environment in which Sega Genesis (MegaDrive outside the US) games may run
with fairly accurate audio and video.

--------------------------------------------------------------------------------
HOW DO I INSTALL IT?
--------------------------------------------------------------------------------
You'll need SDL version 1.0 or greater. Fetch it from
http://www.libsdl.org/, and install it. Make sure, if you're
using prepackaged rpm's or deb's, to get both the SDL and SDL-devel packages.

For Intel/AMD/Cyrix CPUs, you'll also need NASM, which can be found at
http://www.web-sites.co.uk/nasm/, in order to utilize the assembler
optimizations. *WARNING*: The NASM that comes prepackaged with RedHat 7.x is
broken! Install the rpm from RedHat 6.x, and it should work. One such rpm can
be found at http://pknet.com/~joe/nasm-0.98-2.i386.rpm.

Now you should be able to build and install DGen like any other
autoconf-using package:

  sh ./configure && make && make install

If you have egcs or GCC >= 2.95, you can give configure the '--with-extra-opt'
switch, which will add some insane optimization to the CFLAGS used to compile
the program, giving the binary a very decent speed boost.

The control configuration and other options available in DGen/SDL are set
through a file dgenrc, which should be in the location $HOME/.dgen/dgenrc.
The sample.dgenrc file in this archive documents all the fields, and so does the
dgenrc(5) manpage. If it doesn't exist, an empty one will be created when
dgen is run.

Now that it's been installed, running 'dgen ROMFILE' should run the Genesis
program contained in ROMFILE. For a list of all the fancy options supported,
run either 'dgen' with no options, or see the dgen(1) manpage for more detail.

--------------------------------------------------------------------------------
IT'S TOO SLOW!
--------------------------------------------------------------------------------
Well, sorry. Here are some ideas to help accelerate it:

 - Lower the value of "int_nice" in your dgenrc. 0 is good for short tempers :-)
 - Turn off sound.
 - Run fullscreen, if possible.
 - Increase int_soundsegs to 16 or (if you're desperate) 32.
 - Buy a faster computer. (sorry, had to say it ;)

--------------------------------------------------------------------------------
IT'S TOO FAST!
--------------------------------------------------------------------------------
I couldn't believe it either, but someone asked me this. If you happen to find
yourself in this unfortunate predicament, here's how to slow it down:

 - Raise the sound rate to 44100Hz. It sounds a LOT better, and is a LOT slower.
 - Increase the value of int_nice in your dgenrc. This is the best way to slow
   things down, since it makes dgen much more CPU friendly.
 - In the game, press the key set to 'dgen_cpu_toggle' (which defaults to F11),
   to use the Musashi CPU core, which is slower and more accurate than
   StarScream.

--------------------------------------------------------------------------------
FREQUENTLY ASKED QUESTIONS
--------------------------------------------------------------------------------
Q: DGen segfaults on startup!
A: The two most common causes of this are a broken NASM, or an absent NASM.
   RedHat 7.x distributions ship with a prepackaged NASM which doesn't work.
   You'll need to use an RPM from a different distribution, or RedHat 6.x.

Q: How do I set up OpenGL mode?
A: OpenGL mode can be activated either from the commandline, or from the dgenrc
   file. Using the '-G XxY' switch will activate OpenGL mode. The switch takes
   the desired resolution as an argument. For example:
	$ dgen -G 640x480 sonic.bin
   will run with a 640x480 screen resolution.

   So that you do not have to type this switch every time you use dgen, you
   can add the following lines to ~/.dgen/dgenrc so that dgen will start in
   OpenGL mode every time:
   	bool_opengl = yes
	int_opengl_width = 640
	int_opengl_height = 480
   int_opengl_width and int_opengl_height can be changed to suit your own
   preferences.

Q: I make dgen setuid-root, and try to run it fullscreen, and all I get is
   a black screen with the game running in the middle of it.
A: You need to have your X server set up with a 320x240 video mode. For XFree86,
   if you have XF86Setup installed (which is more than likely):
   - Close all X sessions you have running.
   - From a console, as root, run "XF86Setup". When asked "Would you like to use
     the existing XF86Config file for defaults?" say Yes.
   - After 30 seconds or so the X server should start in VGA mode, and put you
     in the setup program. Everything else is set up, just click on the
     "Modeselection" button in the top right-hand corner.
   - From the list of possible modes, choose 1024x768 (or whatever mode you use
     regularly), and 320x240 (which is the mode DGen wants). Choose a bits-per-
     pixel from the list on the bottom (DGen works best in 15- or 16-bpp).
   - Press "Done", then "Okay".
   - It should restart the X server. When you see the message
     "Congratulations!...", try pressing CTRL-ALT-KEYPAD+ to switch to your new
     320x240 mode, just to make sure it works. Press CTRL-ALT-KEYPAD- to switch
     back, and press "Save the configuration and exit".
   - Restart your X session. If you use xdm, press CTRL-ALT-BACKSPACE to kill
     the X server and restart it. Fullscreen should work now.

Q: DGen's colors are messed up!
A: Some video cards, such as most Riva cards, use different pixel formats
   for true-color modes. If you use 16-bpp bit depth, try 15- or 32-bpp bit
   depth.

Q: I try compiling, but when I run ./configure, it tells me it can't find
   sdl-config. I installed SDL from the rpm's.
A: You need to install the SDL-devel rpm as well:
   http://www.devolution.com/~slouken/SDL/release/SDL-devel-<version>-1.i386.rpm

Q: Sonic 2 freezes right before the title screen!
A: There is a bug in the StarScream CPU core which causes Sonic 2 to freeze
   at this point. Press F11 to switch to the Musashi core, until you get to 
   the title sequence, then press it again to switch back to StarScream. Save
   a snapshot here, so you don't have to go through it again. :)

--------------------------------------------------------------------------------
BUGS
--------------------------------------------------------------------------------
Of course the chances of finding a bug are unlikely, but if you do, tell me
about it. Send me a patch too, if possible :-) Also, check the FAQ above before
reporting bugs, to see if your question's already been answered.

Enjoy DGen!
