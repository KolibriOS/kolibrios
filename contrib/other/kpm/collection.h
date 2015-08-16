#ifndef __COLLECTION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

typedef struct
{
    list_t  groups;
    char   *issue;
}collection_t;

typedef struct
{
    list_t list;
    list_t packages;
    char   *name;
}pkg_group_t;

typedef struct package
{
    list_t list;
    int    id;
    char   *name;
    char   *version;
    char   *filename;
    char   *description;
}package_t;

collection_t* load_collection_file(const char *name);
collection_t* load_collection_buffer(const char *buffer);

int build_install_list(list_t *list, collection_t *collection);
int build_download_list(list_t *download, list_t *src);
char *make_cache_path(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* __COLLECTION_H__ */
