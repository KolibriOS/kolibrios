#ifndef __PACKAGE_H__

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
    list_t file_list;
    int    id;
    char   *name;
    char   *version;
    char   *filename;
    char   *description;
}package_t;

static inline void list_del_pkg(package_t *pkg)
{
    list_del(&pkg->list);
    free(pkg->description);
    free(pkg->filename);
    free(pkg->version);
    free(pkg->name);
    free(pkg);
};

collection_t* load_collection_file(const char *name);
collection_t* load_collection_buffer(const char *buffer);

int build_install_list(list_t *list, collection_t *collection);
int build_download_list(list_t *download, list_t *src);
void remove_packages(list_t *install, list_t *missed);

char *make_cache_path(const char *path);

void do_download(list_t *download);

#ifdef __cplusplus
}
#endif

#endif /* __PACKAGE_H__ */

