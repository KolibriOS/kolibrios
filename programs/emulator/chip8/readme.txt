CHIP8 - Chip-8 systems emulator, written specially for KolibriOS in pure FASM from scratch.
Author: rgimad (rustem.gimadutdinov@gmail.com)
License: GNU GPL v2

== Usage ==
Open shell in emulator directory (in Eolite Ctrl+G opens shell).

Type for example:

  chip8 roms/ibm.ch8

== Keys ==

CHIP8 emulator can process 16 keys, they are: 0-9, a, b, c, d, e, f

== ROMs ==

Emulator goes with some ROMs: games, demos, programs, etc.
Some of them you can see below:

=== TETRIS (roms/tetris1.ch8) ===

How to play:
4 key is left rotate
5 - left move
6 - right move

After every 5 lines, the speed increases slightly and peaks at 45 lines.

=== SPACE INVADERS (roms/invaders1.ch8) ===

Very famous game

=== MAZE (roms/maze_alt.ch8) ===
A random maze generator

=== RND (roms/rnd.ch8) ===
Generates and prints out a random number. Press any key to generate one more.

=== IBM (roms/ibm.ch8) ===
Prints out the IBM logo

=== CHIP8 (roms/chip8.ch8) ===
Prints out the CHIP8 logo

More ROMs you can find here: https://github.com/dmatlack/chip8/tree/master/roms
