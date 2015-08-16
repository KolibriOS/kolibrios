#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <kos32sys.h>
#include <sys/kos_io.h>

#include "package.h"
#include "http.h"

#define BUFFSIZE  (64*1024)


char *make_url(const char *name)
{
    static char url_buf[128] = "http://ftp.kolibrios.org/users/Serge/new/OS/";
    strcpy(&url_buf[44], name);
    return url_buf;
};

char *make_tmp_path(const char *path)
{
    static char path_buf[64] = "/tmp0/1/";
    strcpy(&path_buf[8], path);
    return path_buf;
};

char *make_cache_path(const char *path)
{
    static char path_buf[64] = "/kolibrios/kpm/cache/";
    strcpy(&path_buf[21], path);
    return path_buf;
};

int http_load_file(const char *path, const char *url)
{
    http_t *http;
    int     received = 0;
    int     offset = 0;
    int     tail;
    char   *buf;
    int     fd;
    int     i;

    buf = user_alloc(BUFFSIZE);
    for(i = 0; i < 16; i++)
        buf[i*4096] = 0;

    fd = open(path, O_CREAT|O_WRONLY);
    if(fd == -1)
    {
        user_free(buf);
        return 0;
    };

    http = http_get(url, NULL,FLAG_STREAM|FLAG_REUSE_BUFFER, NULL);
    if(http == NULL)
        goto err_get;

    do
    {
        if(http_receive_with_retry(http, 500) == 0)
        {
            int count;

//            if(http->flags & 0xffff0000)
//                break;

            count = http->content_received - received;
            if(count+offset <= BUFFSIZE)
            {
                memcpy(buf+offset, http->content_ptr, count);
                offset+= count;
            }            else
            {
                tail  = count+offset-BUFFSIZE;
                count = BUFFSIZE - offset;
                if(count)
                {
                    memcpy(buf+offset, http->content_ptr, count);
                    offset = 0;
                };

                write(fd, buf, BUFFSIZE);

                if(tail)
                {
                    memcpy(buf, http->content_ptr+count, tail);
                    offset = tail;
                }
            }
            received = http->content_received;
        }
        else break;

    }while( (http->flags & FLAG_GOT_ALL_DATA) == 0);

    if(offset)
    {
        write(fd, buf, offset);
    }

//    ftruncate(fd, received);
    close(fd);

    if(http->content_ptr)
        user_free(http->content_ptr);
    http_free(http);

    user_free(buf);

    return received;

err_get:
    printf("HTTP GET failed\n");
    return received;
}


int main(int argc, char *argv[])
{
    int   count;
    char *cache_path;
    char *tmp_path;

    if(http_init())
        goto err_init;

    tmp_path = make_tmp_path("packages.xml");

    count = http_load_file(tmp_path, make_url("packages.xml"));

    if(count)
    {
        collection_t *collection;
        package_t   *pkg;
        LIST_HEAD(install_list);
        LIST_HEAD(download_list);

        collection = load_collection_file(tmp_path);

        if(collection && build_install_list(&install_list, collection))
        {
            if(build_download_list(&download_list, &install_list))
                do_download(&download_list);

            if(!list_empty(&download_list))
                remove_missing_packages(&install_list, &download_list);

            list_for_each_entry(pkg, &install_list, list)
                printf("install package %s-%s\n", pkg->name, pkg->version);

            set_cwd("/tmp0/1");

            do_install(&install_list);
        };
     }

    return 0;

err_init:
    printf("HTTP library initialization failed\n");
    return -1;
}

int build_install_list(list_t *list, collection_t *collection)
{
    pkg_group_t *gr;
    int count = 0;

    list_for_each_entry(gr, &collection->groups, list)
    {
        package_t   *pkg, *tmp;

        list_for_each_entry(tmp, &gr->packages, list)
        {
            pkg = (package_t*)malloc(sizeof(package_t));

            INIT_LIST_HEAD(&pkg->file_list);
            pkg->id       = tmp->id;
            pkg->name     = strdup(tmp->name);
            pkg->version  = strdup(tmp->version);
            pkg->filename = strdup(tmp->filename);
            pkg->description = strdup(tmp->description);
            list_add_tail(&pkg->list, list);
            count++;
        }
    };
    return count;
}

int build_download_list(list_t *download, list_t *src)
{
    int count = 0;
    char *cache_path;
    package_t   *pkg, *tmp;
    fileinfo_t  info;
    list_for_each_entry(tmp, src, list)
    {
        cache_path = make_cache_path(tmp->filename);

        if( get_fileinfo(cache_path, &info) != 0)
        {
            pkg = (package_t*)malloc(sizeof(package_t));

            INIT_LIST_HEAD(&pkg->file_list);
            pkg->id       = tmp->id;
            pkg->name     = strdup(tmp->name);
            pkg->version  = strdup(tmp->version);
            pkg->filename = strdup(tmp->filename);
            pkg->description = strdup(tmp->description);
            list_add_tail(&pkg->list, download);
            count++;
        };
    }
    return count;
};

void do_download(list_t *download_list)
{
    package_t   *pkg, *tmp;
    char        *cache_path;
    int         count;

    list_for_each_entry_safe(pkg, tmp, download_list, list)
    {
        printf("package %s-%s\n", pkg->name, pkg->version);
        cache_path = make_cache_path(pkg->filename);
        count = http_load_file(cache_path, make_url(pkg->filename));
        printf("%s loaded %d bytes\n",cache_path, count);
        if( !test_archive(cache_path))
            list_del_pkg(pkg);
        else
            unlink(cache_path);
    };
}

void remove_missing_packages(list_t *install, list_t *missed)
{
    package_t   *mpkg, *mtmp, *ipkg, *itmp;

    list_for_each_entry_safe(mpkg, mtmp, missed, list)
    {
        list_for_each_entry_safe(ipkg, itmp, install, list)
        {
            if(ipkg->id == mpkg->id)
            {
                printf("skip missing package %s-%s\n", ipkg->name, ipkg->version);
                list_del_pkg(ipkg);
            };
        }
        list_del_pkg(mpkg);
    };
};


