
char tmp_cursor[4096];

cursor_t *create_cursor(u32 pid, void *src, u32 flags)
{
  void *img;

  cursor_t *cursor = (cursor_t*)CreateObject(pid, 32);

  if(cursor==NULL)
    return 0;
  dbgprintf("create object at %x\n", cursor);

  cursor->magic = 0x53525543;
  cursor->destroy = __destroy_cursor;

  img = rhd_mem_alloc(&rhd,RHD_MEM_FB,64*64*4);
  if(img==NULL)
    goto cleanup;

  dbgprintf("alloc video memory at %x size %x\n",img,64*64*4);
  dbgprintf("offset %x\n", img-rhd.FbBase);
  cursor->base=img;

  if( (u16)flags==LOAD_INDIRECT){
    cursor->hot_x = (u8)(flags>>24);
    cursor->hot_y = (u8)(flags>>16);
    asm __volatile__
    (
      "cld \n\t"
      "rep stosl"
      :
      :"a"(0),"c"(64*64),"D"(img)
    );
    asm __volatile__
    (
  "1: "
      "movl $32, %%ecx \n\t"
      "rep stosl \n\t"
      "addl $128, %%edi \n\t"
      "decl %%ebx \n\t"
      "jnz 1b"
      :
      :"b"(32),"S"(src),"D"(img)
    );
  }
  else {
    cursor->hot_x = *(u16*)((char*)src+10);
    cursor->hot_y = *(u16*)((char*)src+12);
    copy_cursor(img, src);
    dbgprintf("cursor loaded\n");
  }
  return cursor;
cleanup:
  DestroyObject(cursor);
  return NULL;
};

void destroy_cursor(cursor_t *cursor)
{
  if(cursor->base)
    rhd_mem_free(&rhd,RHD_MEM_FB,cursor->base);
  DestroyObject(cursor);
}

void __stdcall r500_SelectCursor(cursor_t *cursor)
{
  CARD32 base;

  base = (CARD32)cursor->base - rhd.FbBase;
  asm __volatile__
  (
    "cli"
  );
  OUTREG (D1CUR_SURFACE_ADDRESS, rhd.FbIntAddress+base);
  OUTREG (D1CUR_HOT_SPOT, (cursor->hot_x<<16)|(cursor->hot_y&0xFFFF));
  asm __volatile__
  (
    "sti"
  );
}

void __stdcall r500_SetCursor(cursor_t *cursor, int x, int y)
{
   CARD32 tmp = (x<<16)|(y & 0xFFFF) ;

   asm __volatile__
   (
    "pushfl \n\t"
    "cli"
   );

   OUTREG(D1CUR_POSITION, tmp);
   OUTREG(D1CUR_CONTROL, 0x301);
   asm __volatile__
   (
    "popfl"
   );
}

void __stdcall r500_CursorRestore(int x, int y)
{};

