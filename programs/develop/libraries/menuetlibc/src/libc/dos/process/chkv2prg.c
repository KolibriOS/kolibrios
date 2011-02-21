/* Copyright (C) 1999 DJ Delorie, see COPYING.DJ for details */
/* Copyright (C) 1997 DJ Delorie, see COPYING.DJ for details */
#include <libc/stubs.h>
#include <stdlib.h>
#include <sys/system.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define STUB_INFO_MAGIC "StubInfoMagic!!"

static _v2_prog_type type;
static int type_initialized = 0;

static
const _v2_prog_type *_check_v2_prog_internal (int pf);

const _v2_prog_type *_check_v2_prog(const char *program, int pf)
{
  const _v2_prog_type *prog_type;

  if (type_initialized && type.stubinfo)
    free(type.stubinfo);
  type_initialized = 1;

  memset(&type, 0, sizeof(type));

  if (program)
  {
    pf = open(program, O_RDONLY|O_BINARY);
    if (pf < 0)
      return &type;
  }

  prog_type = _check_v2_prog_internal(pf);

  if (program)
    close(pf);

  if (prog_type)
    type.valid = 1;
  return &type;
}

static
const _v2_prog_type *_check_v2_prog_internal (int pf)
{
  unsigned short header[5];
  lseek(pf, 0, SEEK_SET);
  if (read(pf, header, sizeof(header)) != sizeof(header))
    return NULL;
  if (header[0] == 0x010b || header[0] == 0x014c)
  {
    unsigned char firstbytes[1];
    unsigned long coffhdr[40];

    /* Seems to be an unstubbed COFF.  See what the first opcode
       is to determine if it's v1.x or v2 COFF (or an impostor).

       FIXME: the code here assumes that any COFF that's not a V1
       can only be V2.  What about other compilers that use COFF?  */
    type.object_format = _V2_OBJECT_FORMAT_COFF;
    if (lseek(pf, 2, 1) < 0
	|| read(pf, coffhdr, sizeof(coffhdr)) != sizeof(coffhdr)
	|| lseek(pf, coffhdr[10 + 5], 0) < 0
	|| read(pf, firstbytes, 1) != 1) /* scnptr */
        /* "Aha! An impostor!" (The Adventure game) */
      type.object_format = _V2_OBJECT_FORMAT_UNKNOWN;
    else if (firstbytes[0] != 0xa3) /* opcode of movl %eax, 0x12345678 (V1) */
           type.version.v.major = 2;
         else
           type.version.v.major = 1;
    type.exec_format = _V2_EXEC_FORMAT_COFF;
  }
  else if (header[0] == 0x5a4d)	/* "MZ" */
  {
    char go32stub[9];
    unsigned long coff_start = (unsigned long)header[2]*512L;
    unsigned long exe_start;
    type.exec_format = _V2_EXEC_FORMAT_EXE;
    if (header[1])
      coff_start += (long)header[1] - 512L;
    exe_start = (unsigned long)header[4]*16L;
    if (lseek(pf, exe_start, SEEK_SET) != exe_start)
      return NULL;
    if (read(pf, go32stub, 8) != 8)
      return NULL;
    go32stub[8] = 0;
    if (strcmp(go32stub, "go32stub") == 0)
    {
      type.version.v.major = 2;
      type.object_format = _V2_OBJECT_FORMAT_COFF;
      type.exec_format = _V2_EXEC_FORMAT_STUBCOFF;
    }
    else
    {
      int stub_offset;
      char magic[16];
      int struct_length;
      unsigned short coff_id;
      type.version.v.major = 1;
      if (lseek(pf, coff_start - 4, SEEK_SET) != coff_start-4)
        return NULL;
      if (read(pf, &stub_offset, 4) != 4)
        return NULL;
      if (read(pf, &coff_id, 2) != 2)
        return NULL;
      if (coff_id == 0x010b || coff_id == 0x014c)
      {
        type.object_format = _V2_OBJECT_FORMAT_COFF;
        type.exec_format = _V2_EXEC_FORMAT_STUBCOFF;
      }
      if (lseek(pf, stub_offset, 0) != stub_offset)
        return NULL;
      if (read(pf, magic, 16) != 16)
        return NULL;
      if (memcmp(STUB_INFO_MAGIC, magic, 16) == 0)
      {
        if (read(pf, &struct_length, 4) != 4)
          return NULL;
        type.stubinfo = (_v1_stubinfo *)malloc(struct_length);
        memcpy(type.stubinfo->magic, magic, 16);
        type.stubinfo->struct_length = struct_length;
        if (read(pf, type.stubinfo->go32, struct_length - 20)
            != struct_length - 20)
          return NULL;
        type.has_stubinfo = 1;
      }
    }
  }
  else if (header[0] == 0x2123)	/* "#!" */
  {
    type.exec_format = _V2_EXEC_FORMAT_UNIXSCRIPT;
  }
  return &type;
}

