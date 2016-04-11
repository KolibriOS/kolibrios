#ifndef KOLIBRI_BUTTON_H
#define KOLIBRI_BUTTON_H

struct kolibri_button {
  unsigned int x65536sizex;
  unsigned int y65536sizey;
  unsigned int color;
  unsigned int identifier;
  unsigned int XY;
};

struct kolibri_button *kolibri_new_button(unsigned int tlx, unsigned int tly, unsigned int sizex, unsigned int sizey, 
					  unsigned int identifier, unsigned int color)
{
  struct kolibri_button* new_button = (struct kolibri_button *)malloc(sizeof(struct kolibri_button));
  new_button -> x65536sizex = (tlx << 16) + sizex;
  new_button -> y65536sizey = (tly << 16) + sizey;
  new_button -> color = color;
  new_button -> identifier = identifier;
  new_button -> XY = 0;
}

void draw_button(struct kolibri_button *some_button)
{
  define_button(some_button -> x65536sizex, some_button -> y65536sizey, some_button -> identifier, some_button -> color);
}

unsigned int kolibri_button_get_identifier(void)
{
  unsigned int identifier;

  __asm__ __volatile__(
		       "int $0x40"
		       :"=a"(identifier)
		       :"a"(17)
		       );
  /* If no button pressed, returns 1 */
  /* Otherwise, returns identifier of button */

  if(identifier != 1) /* Button was detected indeed */
    return identifier>>8;
  else
    return identifier; /* No button detected */
}

#endif /* KOLIBRI_BUTTON_H */
