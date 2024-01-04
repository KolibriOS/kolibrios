/*
 * Copyright (C) KolibriOS team 2004-2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <sys/types.h>
#include <sys/ksys.h>

int read_file(const char *path, void *buff,
               size_t offset, size_t count, size_t *reads)
{
    ksys_file_status_t st = _ksys_file_read(path, offset, count, buff);
    *reads = st.rw_bytes;
    return st.status == KSYS_FS_ERR_EOF ? 0 : st.status;
}
