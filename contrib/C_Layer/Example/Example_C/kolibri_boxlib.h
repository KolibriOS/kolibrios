#ifndef KOLIBRI_BOXLIB_H
#define KOLIBRI_BOXLIB_H

extern int init_boxlib_asm(void); 

int kolibri_boxlib_init(void)
{
  int asm_init_status = init_boxlib_asm();
  
  /* just return asm_init_status? or return init_boxlib_asm() ?*/

  if(asm_init_status == 0)
    return 0;
  else
    return 1;
}

#endif /* KOLIBRI_BOXLIB_H */
