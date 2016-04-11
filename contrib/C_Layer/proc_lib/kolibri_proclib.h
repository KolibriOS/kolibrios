#ifndef KOLIBRI_PROCLIB_H
#define KOLIBRI_PROCLIB_H

int kolibri_proclib_init(void)
{
  int asm_init_status = init_proclib_asm();
  
  /* just return asm_init_status? or return init_proclib_asm() ?*/

  if(asm_init_status == 0)
    return 0;
  else
    return 1;
}

enum Mode {
	OD_OPEN,
	OD_SAVE,
	OD_DIR
};

#endif /* KOLIBRI_PROCLIB_H */
