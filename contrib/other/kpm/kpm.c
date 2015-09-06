#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <kos32sys.h>
#include <sys/kos_io.h>

#include "getopt.h"
#include "package.h"
#include "http.h"

void process_task(list_t *task);

#define BUFFSIZE  (64*1024)

#define OPTION_STD_BASE 150

enum option_values
{
      OPTION_HELP = OPTION_STD_BASE,
      OPTION_LIST_PACKAGES,
      OPTION_LIST_INSTALLED,
      OPTION_INSTALL_ALL
};

static const struct option longopts[] =
{
    {"list-packages", no_argument, NULL, OPTION_LIST_PACKAGES},
    {"list-installed",no_argument, NULL, OPTION_LIST_INSTALLED},
    {"install-all",no_argument, NULL, OPTION_INSTALL_ALL},
    {NULL,0,NULL,0}
};

static void show_usage ()
{
    sprintf (conbuf, "Usage: kpm [option...]\n");
    con_write_asciiz(conbuf);

    sprintf (conbuf, ("\
Options:\n\
  --list-packages\n\
                          show available packages\n"));
    con_write_asciiz(conbuf);

    sprintf (conbuf, ("\
  --list-installed\n\
                          show available packages\n"));
    con_write_asciiz(conbuf);

    sprintf (conbuf, ("\
  --install all\n\
                          install all packages\n"));
    con_write_asciiz(conbuf);
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
    int   act = 1;

    if(http_init())
        goto err_init;

    set_cwd("/tmp0/1");

    con_init(80, 25, 80, 250, "Kolibri package manager");

    tmp_path = make_tmp_path("packages.xml");

    count = http_load_file(tmp_path, make_url("packages.xml"));

    if(count)
        build_server_list(&server_list, tmp_path);

    while(act)
    {
        int val;
        int index;

        val = getopt_long_only(argc, argv,"",longopts, &index);

        switch(val)
        {
            case OPTION_LIST_PACKAGES:
                sprintf(conbuf,"available packages:\n\n");
                con_write_asciiz(conbuf);
                print_pkg_list(&server_list);
                act = 0;
                break;

            case OPTION_LIST_INSTALLED:
                sprintf(conbuf,"installed packages:\n\n");
                con_write_asciiz(conbuf);
                print_pkg_list(&local_list);
                act = 0;
                break;

            case OPTION_INSTALL_ALL:
                copy_list(&task_list, &server_list);
                process_task(&task_list);
                act = 0;
                break;

            default:
                show_usage();
                act = 0;
        }
    };

    con_exit(0);

    return 0;

err_init:
    printf("HTTP library initialization failed\n");
    return -1;
}

