PlayNote (release date 2020.05.17)

PlayNote is a programme to play a note. Sound plays through a sound driver.

Usage: PlayNote <path>
 path - path to a file to be played.

Examples:
 PlayNote note.raw
 PlayNote /tmp0/1/note.raw

===========================
To generate a note in a .wav format with a sox (to listening):
 sox -n -L -c 1 -b 16 -r 48000 Note_C6.wav synth 1 sine 1046.4
To generate a note in a .raw format with a sox (to PlayNote):
 sox -n -L -c 1 -b 16 -r 48000 Note_C6.raw synth 1 sine 1046.4

To install a sox in Ubuntu:
 sudo apt install sox
===========================

//--------------------------------------//
  The programme: 
   - Compiled with KTCC compiler.
   - Written in KolibriOS NB svn7768.
   - Designed and written by JohnXenox
     aka Aleksandr Igorevich.
//--------------------------------------//

