MTAR_ESUCCESS     =  0
MTAR_EFAILURE     = -1
MTAR_EOPENFAIL    = -2
MTAR_EREADFAIL    = -3
MTAR_EWRITEFAIL   = -4
MTAR_ESEEKFAIL    = -5
MTAR_EBADCHKSUM   = -6
MTAR_ENULLRECORD  = -7
MTAR_ENOTFOUND    = -8

MTAR_TREG   = '0'
MTAR_TLNK   = '1'
MTAR_TSYM   = '2'
MTAR_TCHR   = '3'
MTAR_TBLK   = '4'
MTAR_TDIR   = '5'
MTAR_TFIFO  = '6'

struct mtar_header_t
  mode      dd ?
  owner     dd ?
  size      dd ?
  mtime     dd ?
  type      dd ?
  name      rb 100
  linkname  rb 100
ends

struct mtar_t 
  read_func       dd ? 
  write_func      dd ?
  seek_func       dd ?
  close_func      dd ?
  stream          dd ?
  pos             dd ?
  remaining_data  dd ?
  last_header     dd ?
ends