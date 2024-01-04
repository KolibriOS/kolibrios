/*
 * Copyright (C) KolibriOS team 2004-2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <sys/types.h>
#include <errno.h>
#include <sys/ksys.h>

int write_file(const char *path, const void *buff,
               size_t offset, size_t count, size_t *writes)
{
    ksys_file_status_t st = _ksys_file_write(path, offset, count, buff);
    *writes = st.rw_bytes;
    if(!st.status)
        return 0;
    else if (st.status == KSYS_FS_ERR_8)
        return ENOSPC;
    return -1;
}
