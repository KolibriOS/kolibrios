SDL_bdf version 1.2

http://www.geocities.com/andre_leiradella/

For copyright information see the source files.

SDL_bdf is a small library that renders BDF fonts. As of version 1.2, SDL_bdf
doesn't depend on SDL anymore, you can use it with any graphics library throgh
it's API, but the bundled exampled uses SDL to show a text with a BDF font on
the screen.

The library has been tested with SDL under Windows but should work on any
platform/graphics library. The functions provided are:

. BDF_Font *BDF_OpenFont(BDF_ReadByte getbyte, void *info, int *error): Opens a
  BDF font, it receives the function that will produce the stream of bytes, the
  user defined void pointer that will be passed to getbyte and a pointer to an
  int that will receive the error code. Returns the BDF font.
. void BDF_CloseFont(BDF_Font *font): Closes the font and frees all associated
  memory.
. void BDF_SizeH(BDF_Font *font, char *text, int *x0, int *y0, int *width,
  int *height): Determines the size of the horizontal text, returns the width
  and height of the smallest rectangle that can acomodate the rendered text and
  the start position in x0 and y0 on where the text must be rendered to exactly
  fit the rectangle. This is because the render functions take the y parameter
  as the baseline of the text to allow different fonts (e.g. normal and italic)
  to be mixed in the same line. It handles NULL pointers for pieces of
  information you don't want.
. void BDF_SizeEntitiesH(BDF_Font *font, char *text, int *x0, int *y0,
  int *width, int *height): Same as above but accepts entities in the
  form &...; (e.g. Andr&eacute;)
. int BDF_DrawH(void *surface, BDF_PutPixel putpixel, BDF_Font *font,
  char *text, int x, int y, unsigned int color): Draws the text at the given
  surface starting at position (x, y). It calls putpixel with the surface,
  coordinates and color to draw the pixel (doesn't clip). Returns the next x
  coordinate to continue to render more text. Only accepts characters in the
  range [0..255].
. int BDF_DrawEntitiesH(void *, BDF_PutPixel, BDF_Font *, char *, int, int,
  unsigned int): Same as above but accepts entities in the form &...;

TODO:

. Handle vertical writing.
. Use a hash table instead of an ordered array to access the glyphs by name.
. What else? Tell me: leiradella@bigfoot.com

