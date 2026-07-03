/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include <stdio.h>
#include <sys/dirent.h>
#include <sys/ksys.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define READDIR_ENCODING KSYS_FILE_UTF8

#define READDIR_BUF_LEN ((READDIR_ENCODING == KSYS_FILE_CP866) ? 264 : 520)

#define CHECK_DIR_ERR(path, n, data, ...)                                                         \
    data = malloc(sizeof(ksys_readdir_buff_t) + (sizeof(ksys_bdfe_t) + READDIR_BUF_LEN) * n);     \
    if (!data || _ksys_read_dir(path, n, READDIR_ENCODING, data).status) {                         \
        void* _CHECK_DIR_ERR_ARGS[] = { data, __VA_ARGS__ };                                      \
        for (size_t i = 0; i < sizeof(_CHECK_DIR_ERR_ARGS) / sizeof(*_CHECK_DIR_ERR_ARGS); ++i) { \
            if (_CHECK_DIR_ERR_ARGS[i]) {                                                         \
                free(_CHECK_DIR_ERR_ARGS[i]);                                                     \
            }                                                                                     \
        }                                                                                         \
        errno = ENOTDIR;                                                                          \
        return NULL;                                                                              \
    }

DIR* opendir(const char* path)
{
    DIR* list = malloc(sizeof(DIR));
    if (list == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    list->pos = 0;
    unsigned num_of_file = 0;

    ksys_readdir_buff_t* data;

    CHECK_DIR_ERR(path, 2, data, list);

    num_of_file = data->num_of_files;
    free(data);

    list->objs = (struct dirent*)malloc(num_of_file * sizeof(struct dirent));

    CHECK_DIR_ERR(path, num_of_file, data, list->objs, list);

    for (int i = 0; i < num_of_file; i++) {
        ksys_bdfe_t* d = (ksys_bdfe_t*)((char*)(&data->files) + ((sizeof(ksys_bdfe_t) + READDIR_BUF_LEN) * i));
        list->objs[i].d_ino = i;
        list->objs[i].d_type = d->attributes;
        strcpy(list->objs[i].d_name, d->name);
    }
    free(data);

    list->num_objs = num_of_file;
    return list;
}
