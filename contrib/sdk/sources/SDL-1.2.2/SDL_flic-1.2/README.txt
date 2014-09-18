SDL_flic version 1.2

http://www.geocities.com/andre_leiradella/

For copyright information see the source files.

SDL_flic is a small library that renders frames of FLI and FLC animation files.

The library has been tested with under Windows but should work on any platform.
The functions provided are:

. int FLI_Version(void): Returns the library version in the format
  MAJOR << 16 | MINOR.
. FLI_Animation *FLI_Open(SDL_RWops *rwops, int *error): Opens a FLIC animation
  and returns a pointer to it. rwops is left at the same point it was before
  the the call. error receives the result of the call.
. void FLI_Close(FLI_Animation *flic): Closes the animation, closes the stream
  and frees all used memory.
. int FLI_NextFrame(FLI_Animation *flic): Renders the next frame of the
  animation returning an int to indicate if it was successfull or not.
. int FLI_Rewind(FLI_Animation *flic): Rewinds the animation to the first
  frame.
. int FLI_Skip(FLI_Animation *flic): Skips the current frame without rendering
  it.

TODO:

. Handle other formats of FLIC animation.
. Play animation inside a thread?
. What else? Tell me: leiradella@bigfoot.com

