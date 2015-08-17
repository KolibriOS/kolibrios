#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <kos32sys.h>
#include <sys/kos_io.h>

#include "getopt.h"
#include "package.h"
#include "http.h"

#define BUFFSIZE  (64*1024)

extern char conbuf[256];

#define OPTION_STD_BASE 150

enum option_values
{
      OPTION_HELP = OPTION_STD_BASE,
      OPTION_LIST_PACKAGES,
      OPTION_LIST_INSTALLED
};

static const struct option longopts[] =
{
    {"list-packages", no_argument, NULL, OPTION_LIST_PACKAGES},
    {"list-installed",no_argument, NULL, OPTION_LIST_INSTALLED},
    {NULL,0,NULL,0}
};

int main(int argc, char *argv[])
{
    LIST_HEAD(server_list);
    LIST_HEAD(download_list);
    LIST_HEAD(cache_list);
    LIST_HEAD(local_list);
    LIST_HEAD(task_list);

    int   count;
    char *cache_path;
    char *tmp_path;

    if(http_init())
        goto err_init;

    set_cwd("/tmp0/1");

    con_init(80, 25, 80, 250, "Kolibri package manager");

    tmp_path = make_tmp_path("packages.xml");

    count = http_load_file(tmp_path, make_url("packages.xml"));

    if(count)
        build_server_list(&server_list, tmp_path);

    while(1)
    {
        int val;
        int index;

        val = getopt_long_only(argc, argv,"",longopts, &index);

        if(val == -1)
            break;

        switch(val)
        {
            case OPTION_LIST_PACKAGES:
                sprintf(conbuf,"available packages:\n\n");
                con_write_asciiz(conbuf);
                print_pkg_list(&server_list);
                con_exit(0);
                return 0;

            case OPTION_LIST_INSTALLED:
                sprintf(conbuf,"installed packages:\n\n");
                con_write_asciiz(conbuf);
                print_pkg_list(&local_list);
                con_exit(0);
                return 0;

            default:
                break;
        }
    };

#if 0
    {
        package_t   *pkg;
        LIST_HEAD(install_list);
        LIST_HEAD(download_list);

        if(collection && build_install_list(&install_list, collection))
        {
            if(build_download_list(&download_list, &install_list))
                do_download(&download_list);

            if(!list_empty(&download_list))
                remove_missing_packages(&install_list, &download_list);

            list_for_each_entry(pkg, &install_list, list)
            {
                sprintf(conbuf,"install package %s-%s\n", pkg->name, pkg->version);
                con_write_asciiz(conbuf);
            };

            do_install(&install_list);
        };
    }
#endif

    con_exit(0);

    return 0;

err_init:
    printf("HTTP library initialization failed\n");
    return -1;
}




