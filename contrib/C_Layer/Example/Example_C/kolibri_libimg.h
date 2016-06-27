#ifndef KOLIBRI_LIBIMG_H
#define KOLIBRI_LIBIMG_H

int kolibri_libimg_init(void)
{
  int asm_init_status = init_libimg_asm();
  
  /* just return asm_init_status? or return init_libimg_asm() ?*/

  if(asm_init_status == 0)
    return 0;
  else
    return 1;
}

extern void* (*img_decode)(void *, uint32_t, uint32_t) __attribute__((__stdcall__));
extern void* (*img_encode)(void *, uint32_t, uint32_t) __attribute__((__stdcall__));
extern void* (*img_create)(uint32_t, uint32_t, uint32_t) __attribute__((__stdcall__));
extern void (*img_to_rgb2)(void *, void *) __attribute__((__stdcall__));
extern void* (*img_to_rgb)(void *) __attribute__((__stdcall__));
extern uint32_t (*img_flip)(void *, uint32_t) __attribute__((__stdcall__));
extern uint32_t (*img_flip_layer)(void *, uint32_t) __attribute__((__stdcall__));
extern uint32_t (*img_rotate)(void *, uint32_t) __attribute__((__stdcall__));
extern uint32_t (*img_rotate_layer)(void *, uint32_t) __attribute__((__stdcall__));
extern void (*img_draw)(void *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t	) __attribute__((__stdcall__));
extern uint32_t (*img_count)(void *) __attribute__((__stdcall__));
extern uint32_t (*img_destroy)(void *) __attribute__((__stdcall__));
extern uint32_t (*img_destroy_layer)(void *) __attribute__((__stdcall__));

#endif /* KOLIBRI_LIBIMG_H */
