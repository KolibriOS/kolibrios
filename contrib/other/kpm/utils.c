#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <kos32sys.h>
#include <sys/kos_io.h>
#include "package.h"
#include "http.h"

#define BUFFSIZE  (64*1024)

char conbuf[256];


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
            }
            else
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

                sprintf(conbuf, "%d bytes loaded\r", http->content_received);
                con_write_asciiz(conbuf);

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
            pkg->group    = strdup(tmp->group);
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
        sprintf(conbuf,"package %s-%s\n", pkg->name, pkg->version);
        con_write_asciiz(conbuf);
        cache_path = make_cache_path(pkg->filename);
        count = http_load_file(cache_path, make_url(pkg->filename));
        sprintf(conbuf,"%s %d bytes loaded\n",cache_path, count);
        con_write_asciiz(conbuf);

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
                sprintf(conbuf,"skip missing package %s-%s\n", ipkg->name, ipkg->version);
                con_write_asciiz(conbuf);
                list_del_pkg(ipkg);
            };
        }
        list_del_pkg(mpkg);
    };
};

int copy_list(list_t *list, list_t *src)
{
    package_t   *pkg, *tmp;
    int count = 0;

    list_for_each_entry(tmp, src, list)
    {
        pkg = (package_t*)malloc(sizeof(package_t));

        INIT_LIST_HEAD(&pkg->file_list);
        pkg->id       = tmp->id;
        pkg->name     = strdup(tmp->name);
        pkg->version  = strdup(tmp->version);
        pkg->group    = strdup(tmp->group);
        pkg->filename = strdup(tmp->filename);
        pkg->description = strdup(tmp->description);
        list_add_tail(&pkg->list, list);
        count++;
    };
    return count;
}

int build_server_list(list_t *slist, const char *path)
{
    collection_t *collection;
    package_t    *pkg;
    LIST_HEAD(install_list);
    LIST_HEAD(download_list);
    int count = 0;

    collection = load_collection_file(path);

    if(collection)
    {
        package_t   *pkg, *tmp;

        list_for_each_entry(tmp, &collection->packages, list)
        {
            pkg = (package_t*)malloc(sizeof(package_t));

            INIT_LIST_HEAD(&pkg->file_list);
            pkg->id       = tmp->id;
            pkg->name     = strdup(tmp->name);
            pkg->version  = strdup(tmp->version);
            pkg->group    = strdup(tmp->group);
            pkg->filename = strdup(tmp->filename);
            pkg->description = strdup(tmp->description);
            list_add_tail(&pkg->list, slist);
            count++;
        };
    };
    return count;
}

void print_pkg_list(list_t *list)
{
    package_t *pkg;

    list_for_each_entry(pkg, list, list)
    {
        sprintf(conbuf,"%s-%s-%s\n", pkg->name, pkg->version, pkg->group);
        con_write_asciiz(conbuf);
    }
}

void process_task(list_t *task)
{
    LIST_HEAD(download_list);

    if(build_download_list(&download_list, task))
        do_download(&download_list);

    if(!list_empty(&download_list))
        remove_missing_packages(task, &download_list);

    do_install(task);
}
