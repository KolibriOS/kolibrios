#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <kos32sys.h>
#include "collection.h"
#include "http.h"

#define BUFFSIZE  (64*1024)


char *make_url(const char *name)
{
    static char url_buf[128] = "http://ftp.kolibrios.org/users/Serge/new/OS/";
    strcpy(&url_buf[44], name);
    return url_buf;
};

char *make_cache_path(const char *path)
{
    static char path_buf[64] = "/tmp0/1/";
    strcpy(&path_buf[8], path);
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

            if(http->flags & 0xffff0000)
                break;

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

    if(http_init())
        goto err_init;

    cache_path = make_cache_path("packages.xml");

    count = http_load_file(cache_path, make_url("packages.xml"));

    if(count)
    {
        collection_t *collection;
        pkg_group_t *gr;

        collection = load_collection_file(cache_path);

        list_for_each_entry(gr, &collection->groups, list)
        {
            package_t   *pkg;

            list_for_each_entry(pkg, &gr->packages, list)
            {
                printf("package %s-%s\n", pkg->name, pkg->version);
                cache_path = make_cache_path(pkg->filename);
                count = http_load_file(cache_path, make_url(pkg->filename));
                printf("%s loaded %d\n",cache_path, count);
            }
        };
     }

    return 0;

err_init:
    printf("HTTP library initialization failed\n");
    return -1;
}
