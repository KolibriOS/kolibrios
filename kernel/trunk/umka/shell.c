/*
    UMKa - User-Mode KolibriOS developer tools
    umka_shell - interactive shell
    Copyright (C) 2018--2020  Ivan Baravy <dunkaist@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include "getopt.h"
#include "vdisk.h"
#include "umka.h"
#include "trace.h"
#include "pci.h"
#include "lodepng.h"

#define PATH_MAX 4096
#define FGETS_BUF_LEN 4096
#define MAX_COMMAND_ARGS 42
#define PRINT_BYTES_PER_LINE 32
#define MAX_DIRENTS_TO_READ 100
#define MAX_BYTES_TO_READ (1024*1024)

#define DEFAULT_READDIR_ENCODING UTF8
#define DEFAULT_PATH_ENCODING UTF8

FILE *fin, *fout;

char cur_dir[PATH_MAX] = "/";
const char *last_dir = cur_dir;
bool cur_dir_changed = true;

char cmd_buf[FGETS_BUF_LEN];

typedef struct {
    char *name;
    void (*func) (int, char **);
} func_table_t;

const char *f70_status_name[] = {
                                 "success",
                                 "disk_base",
                                 "unsupported_fs",
                                 "unknown_fs",
                                 "partition",
                                 "file_not_found",
                                 "end_of_file",
                                 "memory_pointer",
                                 "disk_full",
                                 "fs_fail",
                                 "access_denied",
                                 "device",
                                 "out_of_memory"
                                };

static const char *
get_f70_status_name(int s) {
    switch (s) {
    case ERROR_SUCCESS:
//        return "";
    case ERROR_DISK_BASE:
    case ERROR_UNSUPPORTED_FS:
    case ERROR_UNKNOWN_FS:
    case ERROR_PARTITION:
    case ERROR_FILE_NOT_FOUND:
    case ERROR_END_OF_FILE:
    case ERROR_MEMORY_POINTER:
    case ERROR_DISK_FULL:
    case ERROR_FS_FAIL:
    case ERROR_ACCESS_DENIED:
    case ERROR_DEVICE:
    case ERROR_OUT_OF_MEMORY:
        return f70_status_name[s];
    default:
        return "unknown";
    }
}

static void
convert_f70_file_attr(uint32_t attr, char s[KF_ATTR_CNT+1]) {
    s[0] = (attr & KF_READONLY) ? 'r' : '-';
    s[1] = (attr & KF_HIDDEN)   ? 'h' : '-';
    s[2] = (attr & KF_SYSTEM)   ? 's' : '-';
    s[3] = (attr & KF_LABEL)    ? 'l' : '-';
    s[4] = (attr & KF_FOLDER)   ? 'f' : '-';
    s[5] = '\0';
}

static void
print_f70_status(f7080ret_t *r, int use_ebx) {
    fprintf(fout, "status = %d %s", r->status, get_f70_status_name(r->status));
    if (use_ebx &&
        (r->status == ERROR_SUCCESS || r->status == ERROR_END_OF_FILE))
        fprintf(fout, ", count = %d", r->count);
    fputc('\n', fout);
}

static bool
parse_uintmax(const char *str, uintmax_t *res) {
    char *endptr;
    *res = strtoumax(str, &endptr, 0);
    bool ok = (str != endptr) && (*endptr == '\0');
    return ok;
}

static bool
parse_uint32(const char *str, uint32_t *res) {
    uintmax_t x;
    if (parse_uintmax(str, &x) && x <= UINT32_MAX) {
        *res = (uint32_t)x;
        return true;
    } else {
        perror("invalid number");
        return false;
    }
}

static bool
parse_uint64(const char *str, uint64_t *res) {
    uintmax_t x;
    if (parse_uintmax(str, &x) && x <= UINT64_MAX) {
        *res = x;
        return true;
    } else {
        perror("invalid number");
        return false;
    }
}

static void
print_bytes(uint8_t *x, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (i % PRINT_BYTES_PER_LINE == 0 && i != 0) {
            fputc('\n', fout);
        }
        fprintf(fout, "%2.2x", x[i]);
    }
    fputc('\n', fout);
}

static void
print_hash(uint8_t *x, size_t len) {
    hash_context ctx;
    hash_oneshot(&ctx, x, len);
    for (size_t i = 0; i < HASH_SIZE; i++) {
        fprintf(fout, "%2.2x", ctx.hash[i]);
    }
    fputc('\n', fout);
}

static const char *
get_last_dir(const char *path) {
    const char *last = strrchr(path, '/');
    if (!last) {
        last = path;
    } else if (last != path || last[1] != '\0') {
        last++;
    }
    return last;
}

static void
prompt() {
    if (cur_dir_changed) {
        if (umka_initialized) {
            COVERAGE_ON();
            umka_sys_get_cwd(cur_dir, PATH_MAX);
            COVERAGE_OFF();
        }
        last_dir = get_last_dir(cur_dir);
        cur_dir_changed = false;
    }
    fprintf(fout, "%s> ", last_dir);
    fflush(fout);
}

static int
next_line(int is_tty) {
    if (is_tty) {
        prompt();
    }
    return fgets(cmd_buf, FGETS_BUF_LEN, fin) != NULL;
}

static int
split_args(char *s, char **argv) {
    int argc = -1;
    for (; (argv[++argc] = strtok(s, " \t\n\r")) != NULL; s = NULL);
    return argc;
}

static void
shell_umka_init(int argc, char **argv) {
    const char *usage = \
        "usage: umka_init";
    (void)argv;
    if (argc < 0) {
        fputs(usage, fout);
        return;
    }
    COVERAGE_ON();
    umka_init();
    COVERAGE_OFF();
}

static void
shell_umka_set_boot_params(int argc, char **argv) {
    const char *usage = \
        "usage: umka_set_boot_params [--x_res <num>] [--y_res <num>]\n"
        "  --x_res <num>    screen width\n"
        "  --y_res <num>    screen height";

    argc -= 1;
    argv += 1;

    while (argc) {
        if (!strcmp(argv[0], "--x_res") && argc > 1) {
            kos_boot.x_res = strtoul(argv[1], NULL, 0);
            kos_boot.pitch = kos_boot.x_res * 4;    // assume 32bpp
            argc -= 2;
            argv += 2;
            continue;
        } else if (!strcmp(argv[0], "--y_res") && argc > 1) {
            kos_boot.y_res = strtoul(argv[1], NULL, 0);
            argc -= 2;
            argv += 2;
            continue;
        } else {
            printf("bad option: %s\n", argv[0]);
            puts(usage);
            exit(1);
        }
    }

}

static void
shell_i40(int argc, char **argv) {
    const char *usage = \
        "usage: i40 <eax> [ebx [ecx [edx [esi [edi [ebp]]]]]]...\n"
        "  see '/kernel/docs/sysfuncs.txt' for details";
    if (argc < 2 || argc > 8) {
        fputs(usage, fout);
        return;
    }
    pushad_t regs = {0, 0, 0, 0, 0, 0, 0, 0};
    if (argv[1]) regs.eax = strtoul(argv[1], NULL, 0);
    if (argv[2]) regs.ebx = strtoul(argv[2], NULL, 0);
    if (argv[3]) regs.ecx = strtoul(argv[3], NULL, 0);
    if (argv[4]) regs.edx = strtoul(argv[4], NULL, 0);
    if (argv[5]) regs.esi = strtoul(argv[5], NULL, 0);
    if (argv[6]) regs.edi = strtoul(argv[6], NULL, 0);
    if (argv[7]) regs.ebp = strtoul(argv[7], NULL, 0);
    COVERAGE_ON();
    umka_i40(&regs);
    COVERAGE_OFF();
    fprintf(fout, "eax = %8.8x  %" PRIu32 "  %" PRIi32 "\n"
           "ebx = %8.8x  %" PRIu32 "  %" PRIi32 "\n",
           regs.eax, regs.eax, (int32_t)regs.eax,
           regs.ebx, regs.ebx, (int32_t)regs.ebx);
}

static void
disk_list_partitions(disk_t *d) {
    for (size_t i = 0; i < d->num_partitions; i++) {
        fprintf(fout, "/%s/%d: ", d->name, i+1);
        if (d->partitions[i]->fs_user_functions == xfs_user_functions) {
            fputs("xfs\n", fout);
        } else if (d->partitions[i]->fs_user_functions == ext_user_functions) {
            fputs("ext\n", fout);
        } else if (d->partitions[i]->fs_user_functions == fat_user_functions) {
            fputs("fat\n", fout);
        } else if (d->partitions[i]->fs_user_functions == ntfs_user_functions) {
            fputs("ntfs\n", fout);
        } else {
            fputs("???\n", fout);
        }
    }
}

static void
shell_ramdisk_init(int argc, char **argv) {
    const char *usage = \
        "usage: ramdisk_init <file>\n"
        "  <file>           absolute or relative path";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    const char *fname = argv[1];
    FILE *f = fopen(fname, "r");
    if (!f) {
        fprintf(fout, "[!] can't open file '%s': %s\n", fname, strerror(errno));
        return;
    }
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    if (fsize > 2880*512) {
        fprintf(fout, "[!] file '%s' is too big, max size is 1474560 bytes\n",
                fname);
        return;
    }
    rewind(f);
    fread(kos_ramdisk, fsize, 1, f);
    fclose(f);
    COVERAGE_ON();
    void *ramdisk = kos_ramdisk_init();
    COVERAGE_OFF();
    disk_list_partitions(ramdisk);
}

static void
shell_disk_add(int argc, char **argv) {
    const char *usage = \
        "usage: disk_add <file> <name> [option]...\n"
        "  <file>           absolute or relative path\n"
        "  <name>           disk name, e.g. hd0 or rd\n"
        "  -c cache size    size of disk cache in bytes";
    if (argc < 3) {
        fputs(usage, fout);
        return;
    }
    size_t cache_size = 0;
    int adjust_cache_size = 0;
    int opt;
    optind = 1;
    const char *file_name = argv[optind++];
    const char *disk_name = argv[optind++];
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
        case 'c':
            cache_size = strtoul(optarg, NULL, 0);
            adjust_cache_size = 1;
            break;
        default:
            fputs(usage, fout);
            return;
        }
    }

    void *userdata = vdisk_init(file_name, adjust_cache_size, cache_size);
    if (userdata) {
        COVERAGE_ON();
        void *vdisk = disk_add(&vdisk_functions, disk_name, userdata, 0);
        COVERAGE_OFF();
        if (vdisk) {
            COVERAGE_ON();
            disk_media_changed(vdisk, 1);
            COVERAGE_OFF();
            disk_list_partitions(vdisk);
            return;
        }
    }
    fprintf(fout, "umka: can't add file '%s' as disk '%s'\n", file_name,
            disk_name);
    return;
}

static void
disk_del_by_name(const char *name) {
    for(disk_t *d = disk_list.next; d != &disk_list; d = d->next) {
        if (!strcmp(d->name, name)) {
            COVERAGE_ON();
            disk_del(d);
            COVERAGE_OFF();
            return;
        }
    }
    fprintf(fout, "umka: can't find disk '%s'\n", name);
}

static void
shell_disk_del(int argc, char **argv) {
    const char *usage = \
        "usage: disk_del <name>\n"
        "  name             disk name, i.e. rd or hd0";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    const char *name = argv[1];
    disk_del_by_name(name);
    return;
}

static void
shell_pwd(int argc, char **argv) {
    const char *usage = \
        "usage: pwd";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    (void)argv;
    bool quoted = false;
    const char *quote = quoted ? "'" : "";
    COVERAGE_ON();
    umka_sys_get_cwd(cur_dir, PATH_MAX);
    COVERAGE_OFF();
    fprintf(fout, "%s%s%s\n", quote, cur_dir, quote);
}

static void
shell_set_pixel(int argc, char **argv) {
    const char *usage = \
        "usage: set_pixel <x> <y> <color> [-i]\n"
        "  x                x window coordinate\n"
        "  y                y window coordinate\n"
        "  color            argb in hex\n"
        "  -i               inverted color";
    if (argc < 4) {
        fputs(usage, fout);
        return;
    }
    size_t x = strtoul(argv[1], NULL, 0);
    size_t y = strtoul(argv[2], NULL, 0);
    uint32_t color = strtoul(argv[3], NULL, 16);
    int invert = (argc == 5) && !strcmp(argv[4], "-i");
    COVERAGE_ON();
    umka_sys_set_pixel(x, y, color, invert);
    COVERAGE_OFF();
}

static void
shell_write_text(int argc, char **argv) {
    const char *usage = \
        "usage: write_text <x> <y> <color> <string> <asciiz> <fill_bg>"
            " <font_and_enc> <draw_to_buf> <scale_factor> <length>"
            " <bg_color_or_buf>\n"
        "  x                x window coordinate\n"
        "  y                y window coordinate\n"
        "  color            argb in hex\n"
        "  string           escape spaces\n"
        "  asciiz           1 if the string is zero-terminated\n"
        "  fill_bg          fill text background with specified color\n"
        "  font_and_enc     font size and string encoding\n"
        "  draw_to_buf      draw to the buffer pointed to by the next param\n"
        "  length           length of the string if it is non-asciiz\n"
        "  bg_color_or_buf  argb or pointer";
    if (argc != 12) {
        fputs(usage, fout);
        return;
    }
    size_t x = strtoul(argv[1], NULL, 0);
    size_t y = strtoul(argv[2], NULL, 0);
    uint32_t color = strtoul(argv[3], NULL, 16);
    const char *string = argv[4];
    int asciiz = strtoul(argv[5], NULL, 0);
    int fill_background = strtoul(argv[6], NULL, 0);
    int font_and_encoding = strtoul(argv[7], NULL, 0);
    int draw_to_buffer = strtoul(argv[8], NULL, 0);
    int scale_factor = strtoul(argv[9], NULL, 0);
    int length = strtoul(argv[10], NULL, 0);
    int background_color_or_buffer = strtoul(argv[11], NULL, 0);
    COVERAGE_ON();
    umka_sys_write_text(x, y, color, asciiz, fill_background, font_and_encoding,
                        draw_to_buffer, scale_factor, string, length,
                        background_color_or_buffer);
    COVERAGE_OFF();
}

static void
shell_dump_win_stack(int argc, char **argv) {
    const char *usage = \
        "usage: dump_win_stack [count]\n"
        "  count            how many items to dump";
    if (argc < 1) {
        fputs(usage, fout);
        return;
    }
    int depth = 5;
    if (argc > 1) {
        depth = strtol(argv[1], NULL, 0);
    }
    for (int i = 0; i < depth; i++) {
        fprintf(fout, "%3i: %3u\n", i, kos_win_stack[i]);
    }
}

static void
shell_dump_win_pos(int argc, char **argv) {
    const char *usage = \
        "usage: dump_win_pos [count]\n"
        "  count            how many items to dump";
    if (argc < 1) {
        fputs(usage, fout);
        return;
    }
    int depth = 5;
    if (argc > 1) {
        depth = strtol(argv[1], NULL, 0);
    }
    for (int i = 0; i < depth; i++) {
        fprintf(fout, "%3i: %3u\n", i, kos_win_pos[i]);
    }
}

static void
shell_dump_win_map(int argc, char **argv) {
    const char *usage = \
        "usage: dump_win_map";
    (void)argv;
    if (argc < 0) {
        fputs(usage, fout);
        return;
    }
    for (size_t y = 0; y < kos_display.height; y++) {
        for (size_t x = 0; x < kos_display.width; x++) {
            fputc(kos_display.win_map[y * kos_display.width + x] + '0', fout);
        }
        fputc('\n', fout);
    }
}

static void
shell_dump_appdata(int argc, char **argv) {
    const char *usage = \
        "usage: dump_appdata <index> [-p]\n"
        "  index            index into appdata array to dump\n"
        "  -p               print fields that are pointers";
    if (argc < 2) {
        fputs(usage, fout);
        return;
    }
    int show_pointers = 0;
    int idx = strtol(argv[1], NULL, 0);
    if (argc > 2 && !strcmp(argv[2], "-p")) {
        show_pointers = 1;
    }
    appdata_t *a = kos_slot_base + idx;
    fprintf(fout, "app_name: %s\n", a->app_name);
    if (show_pointers) {
        fprintf(fout, "process: %p\n", (void*)a->process);
        fprintf(fout, "fpu_state: %p\n", (void*)a->fpu_state);
        fprintf(fout, "exc_handler: %p\n", (void*)a->exc_handler);
    }
    fprintf(fout, "except_mask: %" PRIx32 "\n", a->except_mask);
    if (show_pointers) {
        fprintf(fout, "pl0_stack: %p\n", (void*)a->pl0_stack);
        fprintf(fout, "cursor: %p\n", (void*)a->cursor);
        fprintf(fout, "fd_ev: %p\n", (void*)a->fd_ev);
        fprintf(fout, "bk_ev: %p\n", (void*)a->bk_ev);
        fprintf(fout, "fd_obj: %p\n", (void*)a->fd_obj);
        fprintf(fout, "bk_obj: %p\n", (void*)a->bk_obj);
        fprintf(fout, "saved_esp: %p\n", (void*)a->saved_esp);
    }
    fprintf(fout, "dbg_state: %u\n", a->dbg_state);
    fprintf(fout, "cur_dir: %s\n", a->cur_dir);
    fprintf(fout, "draw_bgr_x: %u\n", a->draw_bgr_x);
    fprintf(fout, "draw_bgr_y: %u\n", a->draw_bgr_y);
    fprintf(fout, "event_mask: %" PRIx32 "\n", a->event_mask);
    fprintf(fout, "terminate_protection: %u\n", a->terminate_protection);
    fprintf(fout, "keyboard_mode: %u\n", a->keyboard_mode);
    fprintf(fout, "captionEncoding: %u\n", a->captionEncoding);
    fprintf(fout, "exec_params: %s\n", a->exec_params);
    fprintf(fout, "wnd_caption: %s\n", a->wnd_caption);
    fprintf(fout, "wnd_clientbox (ltwh): %u %u %u %u\n", a->wnd_clientbox.left,
            a->wnd_clientbox.top, a->wnd_clientbox.width,
            a->wnd_clientbox.height);
    fprintf(fout, "priority: %u\n", a->priority);

    fprintf(fout, "in_schedule: prev");
    if (show_pointers) {
        fprintf(fout, " %p", (void*)a->in_schedule.prev);
    }
    fprintf(fout, " (%u), next",
            (appdata_t*)a->in_schedule.prev - kos_slot_base);
    if (show_pointers) {
        fprintf(fout, " %p", (void*)a->in_schedule.next);
    }
    fprintf(fout, " (%u)\n",
            (appdata_t*)a->in_schedule.next - kos_slot_base);
}

static void
shell_dump_taskdata(int argc, char **argv) {
    const char *usage = \
        "usage: dump_taskdata <index>\n"
        "  index            index into taskdata array to dump";
    if (argc < 2) {
        fputs(usage, fout);
        return;
    }
    int idx = strtol(argv[1], NULL, 0);
    taskdata_t *t = kos_task_table + idx;
    fprintf(fout, "event_mask: %" PRIx32 "\n", t->event_mask);
    fprintf(fout, "pid: %" PRId32 "\n", t->pid);
    fprintf(fout, "state: 0x%" PRIx8 "\n", t->state);
    fprintf(fout, "wnd_number: %" PRIu8 "\n", t->wnd_number);
    fprintf(fout, "counter_sum: %" PRIu32 "\n", t->counter_sum);
    fprintf(fout, "counter_add: %" PRIu32 "\n", t->counter_add);
    fprintf(fout, "cpu_usage: %" PRIu32 "\n", t->cpu_usage);
}

static void
shell_switch_to_thread(int argc, char **argv) {
    const char *usage = \
        "usage: switch_to_thread <tid>\n"
        "  <tid>          thread id to switch to";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    uint8_t tid = strtoul(argv[1], NULL, 0);
    kos_current_slot_idx = tid;
    kos_task_base = kos_task_table + tid;
    kos_current_slot = kos_slot_base + tid;
}

static void
shell_set(int argc, char **argv) {
    const char *usage = \
        "usage: set <var> <value>\n"
        "  <var>          variable to set\n"
        "  <value>        decimal or hex value";
    if (argc != 3) {
        fputs(usage, fout);
        return;
    }
    const char *var = argv[1];
    size_t value = strtoul(argv[2], NULL, 0);
    if (!strcmp(var, "redraw_background")) {
        kos_redraw_background = value;
    } else {
        printf("bad option: %s\n", argv[0]);
        puts(usage);
        exit(1);
    }
}

static void
shell_new_sys_thread(int argc, char **argv) {
    const char *usage = \
        "usage: new_sys_thread";
    if (!argc) {
        fputs(usage, fout);
        return;
    }
    (void)argv;
    size_t tid = umka_new_sys_threads(0, NULL, NULL);
    fprintf(fout, "tid: %u\n", tid);
}

static void
shell_mouse_move(int argc, char **argv) {
    const char *usage = \
        "usage: mouse_move [-l] [-m] [-r] [-x {+|-|=}<value>]"
            "[-y {+|-|=}<value>] [-h {+|-}<value>] [-v {+|-}<value>]\n"
        "  -l             left button is held\n"
        "  -m             middle button is held\n"
        "  -r             right button is held\n"
        "  -x             increase, decrease or set x coordinate\n"
        "  -y             increase, decrease or set y coordinate\n"
        "  -h             scroll horizontally\n"
        "  -v             scroll vertically\n";
    if (!argc) {
        fputs(usage, fout);
        return;
    }
    int lbheld = 0, mbheld = 0, rbheld = 0, xabs = 0, yabs = 0;
    int32_t xmoving = 0, ymoving = 0, hscroll = 0, vscroll = 0;
    int opt;
    optind = 1;
    while ((opt = getopt(argc, argv, "lmrx:y:h:v:")) != -1) {
        switch (opt) {
        case 'l':
            lbheld = 1;
            break;
        case 'm':
            mbheld = 1;
            break;
        case 'r':
            rbheld = 1;
            break;
        case 'x':
            switch (*optarg++) {
            case '=':
                xabs = 1;
                __attribute__ ((fallthrough));
            case '+':
                xmoving = strtol(optarg, NULL, 0);
                break;
            case '-':
                xmoving = -strtol(optarg, NULL, 0);
                break;
            default:
                fputs(usage, fout);
                return;
            }
            break;
        case 'y':
            switch (*optarg++) {
            case '=':
                yabs = 1;
                __attribute__ ((fallthrough));
            case '+':
                ymoving = strtol(optarg, NULL, 0);
                break;
            case '-':
                ymoving = -strtol(optarg, NULL, 0);
                break;
            default:
                fputs(usage, fout);
                return;
            }
            break;
        case 'h':
            if ((optarg[0] != '+') && (optarg[0] != '-')) {
                fputs(usage, fout);
                return;
            }
            hscroll = strtol(optarg, NULL, 0);
            break;
        case 'v':
            if ((optarg[0] != '+') && (optarg[0] != '-')) {
                fputs(usage, fout);
                return;
            }
            vscroll = strtol(optarg, NULL, 0);
            break;
        default:
            fputs(usage, fout);
            return;
        }
    }
    COVERAGE_ON();
    umka_mouse_move(lbheld, mbheld, rbheld, xabs, xmoving, yabs, ymoving,
                    hscroll, vscroll);
    COVERAGE_OFF();
}

static void
shell_process_info(int argc, char **argv) {
    const char *usage = \
        "usage: process_info <pid>\n"
        "  pid              process id to dump, -1 for self";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    process_information_t info;
    int32_t pid = strtol(argv[1], NULL, 0);
    COVERAGE_ON();
    umka_sys_process_info(pid, &info);
    COVERAGE_OFF();
    fprintf(fout, "cpu_usage: %u\n", info.cpu_usage);
    fprintf(fout, "window_stack_position: %u\n", info.window_stack_position);
    fprintf(fout, "window_stack_value: %u\n", info.window_stack_value);
    fprintf(fout, "process_name: %s\n", info.process_name);
    fprintf(fout, "memory_start: 0x%.8" PRIx32 "\n", info.memory_start);
    fprintf(fout, "used_memory: %u (0x%x)\n", info.used_memory,
            info.used_memory);
    fprintf(fout, "pid: %u\n", info.pid);
    fprintf(fout, "box: %u %u %u %u\n", info.box.left, info.box.top,
            info.box.width, info.box.height);
    fprintf(fout, "slot_state: %u\n", info.slot_state);
    fprintf(fout, "client_box: %u %u %u %u\n", info.client_box.left,
            info.client_box.top, info.client_box.width, info.client_box.height);
    fprintf(fout, "wnd_state: 0x%.2" PRIx8 "\n", info.wnd_state);
}

static void
shell_display_number(int argc, char **argv) {
    const char *usage = \
        "usage: display_number <is_pointer> <base> <num_digits> <is_qword>"
            " <show_lead_zeros> <num_or_ptr> <x> <y> <color> <fill_bg> <font>"
            " <draw_to_buf> <scale_factor> <bg_color_or_buf>\n"
        "  is_pointer       if num_or_ptr argument is a pointer\n"
        "  base             0 - dec, 1 - hex, 2 - bin\n"
        "  num_digits       how many digits to print\n"
        "  is_qword         if 1, is_pointer = 1 and num_or_ptr is pointer\n"
        "  show_lead_zeros  0/1\n"
        "  num_or_ptr       number itself or a pointer to it\n"
        "  x                x window coord\n"
        "  y                y window coord\n"
        "  color            argb in hex\n"
        "  fill_bg          0/1\n"
        "  font             0 = 6x9, 1 = 8x16\n"
        "  draw_to_buf      0/1\n"
        "  scale_factor     0 = x1, ..., 7 = x8\n"
        "  bg_color_or_buf  depending on flags fill_bg and draw_to_buf";
    if (argc != 15) {
        fputs(usage, fout);
        return;
    }
    int is_pointer = strtoul(argv[1], NULL, 0);
    int base = strtoul(argv[2], NULL, 0);
    if (base == 10) base = 0;
    else if (base == 16) base = 1;
    else if (base == 2) base = 2;
    else base = 0;
    size_t digits_to_display = strtoul(argv[3], NULL, 0);
    int is_qword = strtoul(argv[4], NULL, 0);
    int show_leading_zeros = strtoul(argv[5], NULL, 0);
    uintptr_t number_or_pointer = strtoul(argv[6], NULL, 0);
    size_t x = strtoul(argv[7], NULL, 0);
    size_t y = strtoul(argv[8], NULL, 0);
    uint32_t color = strtoul(argv[9], NULL, 16);
    int fill_background = strtoul(argv[10], NULL, 0);
    int font = strtoul(argv[11], NULL, 0);
    int draw_to_buffer = strtoul(argv[12], NULL, 0);
    int scale_factor = strtoul(argv[13], NULL, 0);
    uintptr_t background_color_or_buffer = strtoul(argv[14], NULL, 16);
    COVERAGE_ON();
    umka_sys_display_number(is_pointer, base, digits_to_display, is_qword,
                            show_leading_zeros, number_or_pointer, x, y, color,
                            fill_background, font, draw_to_buffer, scale_factor,
                            background_color_or_buffer);
    COVERAGE_OFF();
}

static void
shell_set_window_colors(int argc, char **argv) {
    const char *usage = \
        "usage: set_window_colors <frame> <grab> <work_3d_dark> <work_3d_light>"
            " <grab_text> <work> <work_button> <work_button_text> <work_text>"
            " <work_graph>\n"
        "  *                all colors are in hex";
    if (argc != (1 + sizeof(system_colors_t)/4)) {
        fputs(usage, fout);
        return;
    }
    system_colors_t colors;
    colors.frame            = strtoul(argv[1], NULL, 16);
    colors.grab             = strtoul(argv[2], NULL, 16);
    colors.work_3d_dark     = strtoul(argv[3], NULL, 16);
    colors.work_3d_light    = strtoul(argv[4], NULL, 16);
    colors.grab_text        = strtoul(argv[5], NULL, 16);
    colors.work             = strtoul(argv[6], NULL, 16);
    colors.work_button      = strtoul(argv[7], NULL, 16);
    colors.work_button_text = strtoul(argv[8], NULL, 16);
    colors.work_text        = strtoul(argv[9], NULL, 16);
    colors.work_graph       = strtoul(argv[10], NULL, 16);
    COVERAGE_ON();
    umka_sys_set_window_colors(&colors);
    COVERAGE_OFF();
}

static void
shell_get_window_colors(int argc, char **argv) {
    const char *usage = \
        "usage: get_window_colors";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    (void)argv;
    system_colors_t colors;
    memset(&colors, 0xaa, sizeof(colors));
    COVERAGE_ON();
    umka_sys_get_window_colors(&colors);
    COVERAGE_OFF();
    fprintf(fout, "0x%.8" PRIx32 " frame\n", colors.frame);
    fprintf(fout, "0x%.8" PRIx32 " grab\n", colors.grab);
    fprintf(fout, "0x%.8" PRIx32 " work_3d_dark\n", colors.work_3d_dark);
    fprintf(fout, "0x%.8" PRIx32 " work_3d_light\n", colors.work_3d_light);
    fprintf(fout, "0x%.8" PRIx32 " grab_text\n", colors.grab_text);
    fprintf(fout, "0x%.8" PRIx32 " work\n", colors.work);
    fprintf(fout, "0x%.8" PRIx32 " work_button\n", colors.work_button);
    fprintf(fout, "0x%.8" PRIx32 " work_button_text\n",
            colors.work_button_text);
    fprintf(fout, "0x%.8" PRIx32 " work_text\n", colors.work_text);
    fprintf(fout, "0x%.8" PRIx32 " work_graph\n", colors.work_graph);
}

static void
shell_get_skin_height(int argc, char **argv) {
    const char *usage = \
        "usage: get_skin_height";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    (void)argv;
    COVERAGE_ON();
    uint32_t skin_height = umka_sys_get_skin_height();
    COVERAGE_OFF();
    fprintf(fout, "%" PRIu32 "\n", skin_height);
}

static void
shell_get_screen_area(int argc, char **argv) {
    const char *usage = \
        "usage: get_screen_area";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    (void)argv;
    rect_t wa;
    COVERAGE_ON();
    umka_sys_get_screen_area(&wa);
    COVERAGE_OFF();
    fprintf(fout, "%" PRIu32 " left\n", wa.left);
    fprintf(fout, "%" PRIu32 " top\n", wa.top);
    fprintf(fout, "%" PRIu32 " right\n", wa.right);
    fprintf(fout, "%" PRIu32 " bottom\n", wa.bottom);
}

static void
shell_set_screen_area(int argc, char **argv) {
    const char *usage = \
        "usage: set_screen_area <left> <top> <right> <bottom>\n"
        "  left             left x coord\n"
        "  top              top y coord\n"
        "  right            right x coord (not length!)\n"
        "  bottom           bottom y coord";
    if (argc != 5) {
        fputs(usage, fout);
        return;
    }
    rect_t wa;
    wa.left   = strtoul(argv[1], NULL, 0);
    wa.top    = strtoul(argv[2], NULL, 0);
    wa.right  = strtoul(argv[3], NULL, 0);
    wa.bottom = strtoul(argv[4], NULL, 0);
    COVERAGE_ON();
    umka_sys_set_screen_area(&wa);
    COVERAGE_OFF();
}

static void
shell_get_skin_margins(int argc, char **argv) {
    const char *usage = \
        "usage: get_skin_margins";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    (void)argv;
    rect_t wa;
    COVERAGE_ON();
    umka_sys_get_skin_margins(&wa);
    COVERAGE_OFF();
    fprintf(fout, "%" PRIu32 " left\n", wa.left);
    fprintf(fout, "%" PRIu32 " top\n", wa.top);
    fprintf(fout, "%" PRIu32 " right\n", wa.right);
    fprintf(fout, "%" PRIu32 " bottom\n", wa.bottom);
}

static void
shell_set_button_style(int argc, char **argv) {
    const char *usage = \
        "usage: set_button_style <style>\n"
        "  style            0 - flat, 1 - 3d";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    uint32_t style = strtoul(argv[1], NULL, 0);
    COVERAGE_ON();
    umka_sys_set_button_style(style);
    COVERAGE_OFF();
}

static void
shell_set_skin(int argc, char **argv) {
    const char *usage = \
        "usage: set_skin <path>\n"
        "  path             i.e. /rd/1/DEFAULT.SKN";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    const char *path = argv[1];
    COVERAGE_ON();
    int32_t status = umka_sys_set_skin(path);
    COVERAGE_OFF();
    fprintf(fout, "status: %" PRIi32 "\n", status);
}

static void
shell_get_font_smoothing(int argc, char **argv) {
    const char *usage = \
        "usage: get_font_smoothing";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    (void)argv;
    const char *names[] = {"off", "anti-aliasing", "subpixel"};
    COVERAGE_ON();
    int type = umka_sys_get_font_smoothing();
    COVERAGE_OFF();
    fprintf(fout, "font smoothing: %i - %s\n", type, names[type]);
}

static void
shell_set_font_smoothing(int argc, char **argv) {
    const char *usage = \
        "usage: set_font_smoothing <mode>\n"
        "  mode             0 - off, 1 - gray AA, 2 - subpixel AA";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    int type = strtol(argv[1], NULL, 0);
    COVERAGE_ON();
    umka_sys_set_font_smoothing(type);
    COVERAGE_OFF();
}

static void
shell_get_font_size(int argc, char **argv) {
    const char *usage = \
        "usage: get_font_size";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    (void)argv;
    COVERAGE_ON();
    size_t size = umka_sys_get_font_size();
    COVERAGE_OFF();
    fprintf(fout, "%upx\n", size);
}

static void
shell_set_font_size(int argc, char **argv) {
    const char *usage = \
        "usage: set_font_size <size>\n"
        "  size             in pixels";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    uint32_t size = strtoul(argv[1], NULL, 0);
    COVERAGE_ON();
    umka_sys_set_font_size(size);
    COVERAGE_OFF();
}

static void
shell_button(int argc, char **argv) {
    const char *usage = \
        "usage: button <x> <xsize> <y> <ysize> <id> <color> <draw_button>"
            " <draw_frame>\n"
        "  x                x\n"
        "  xsize            may be size-1, check it\n"
        "  y                y\n"
        "  ysize            may be size-1, check it\n"
        "  id               24-bit\n"
        "  color            hex\n"
        "  draw_button      0/1\n"
        "  draw_frame       0/1";
    if (argc != 9) {
        fputs(usage, fout);
        return;
    }
    size_t x     = strtoul(argv[1], NULL, 0);
    size_t xsize = strtoul(argv[2], NULL, 0);
    size_t y     = strtoul(argv[3], NULL, 0);
    size_t ysize = strtoul(argv[4], NULL, 0);
    uint32_t button_id = strtoul(argv[5], NULL, 0);
    uint32_t color = strtoul(argv[6], NULL, 16);
    int draw_button = strtoul(argv[7], NULL, 0);
    int draw_frame = strtoul(argv[8], NULL, 0);
    COVERAGE_ON();
    umka_sys_button(x, xsize, y, ysize, button_id, draw_button, draw_frame,
                    color);
    COVERAGE_OFF();
}

static void
shell_put_image(int argc, char **argv) {
    const char *usage = \
        "usage: put_image <file> <xsize> <ysize> <x> <y>\n"
        "  file             file with rgb triplets\n"
        "  xsize            x size\n"
        "  ysize            y size\n"
        "  x                x coord\n"
        "  y                y coord";
    if (argc != 6) {
        fputs(usage, fout);
        return;
    }
    FILE *f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    rewind(f);
    uint8_t *image = (uint8_t*)malloc(fsize);
    fread(image, fsize, 1, f);
    fclose(f);
    size_t xsize = strtoul(argv[2], NULL, 0);
    size_t ysize = strtoul(argv[3], NULL, 0);
    size_t x = strtoul(argv[4], NULL, 0);
    size_t y = strtoul(argv[5], NULL, 0);
    COVERAGE_ON();
    umka_sys_put_image(image, xsize, ysize, x, y);
    COVERAGE_OFF();
    free(image);
}

static void
shell_put_image_palette(int argc, char **argv) {
    const char *usage = \
        "usage: put_image_palette <file> <xsize> <ysize> <x> <y> <bpp>"
            " <row_offset>\n"
        "  file             path/to/file, contents according tp bpp argument\n"
        "  xsize            x size\n"
        "  ysize            y size\n"
        "  x                x coord\n"
        "  y                y coord\n"
        "  bpp              bits per pixel\n"
        "  row_offset       in bytes";
    if (argc != 8) {
        fputs(usage, fout);
        return;
    }
    FILE *f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    rewind(f);
    uint8_t *image = (uint8_t*)malloc(fsize);
    fread(image, fsize, 1, f);
    fclose(f);
    size_t xsize = strtoul(argv[2], NULL, 0);
    size_t ysize = strtoul(argv[3], NULL, 0);
    size_t x = strtoul(argv[4], NULL, 0);
    size_t y = strtoul(argv[5], NULL, 0);
    size_t bpp = strtoul(argv[6], NULL, 0);
    void *palette = NULL;
    size_t row_offset = strtoul(argv[7], NULL, 0);
    COVERAGE_ON();
    umka_sys_put_image_palette(image, xsize, ysize, x, y, bpp, palette,
                               row_offset);
    COVERAGE_OFF();
    free(image);
}

static void
shell_draw_rect(int argc, char **argv) {
    const char *usage = \
        "usage: draw_rect <x> <xsize> <y> <ysize> <color> [-g]\n"
        "  x                x coord\n"
        "  xsize            x size\n"
        "  y                y coord\n"
        "  ysize            y size\n"
        "  color            in hex\n"
        "  -g               0/1 - gradient";
    if (argc < 6) {
        fputs(usage, fout);
        return;
    }
    size_t x     = strtoul(argv[1], NULL, 0);
    size_t xsize = strtoul(argv[2], NULL, 0);
    size_t y     = strtoul(argv[3], NULL, 0);
    size_t ysize = strtoul(argv[4], NULL, 0);
    uint32_t color = strtoul(argv[5], NULL, 16);
    int gradient = (argc == 7) && !strcmp(argv[6], "-g");
    COVERAGE_ON();
    umka_sys_draw_rect(x, xsize, y, ysize, color, gradient);
    COVERAGE_OFF();
}

static void
shell_get_screen_size(int argc, char **argv) {
    const char *usage = \
        "usage: get_screen_size";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    (void)argv;
    uint32_t xsize, ysize;
    COVERAGE_ON();
    umka_sys_get_screen_size(&xsize, &ysize);
    COVERAGE_OFF();
    fprintf(fout, "%" PRIu32 "x%" PRIu32 "\n", xsize, ysize);
}

static void
shell_draw_line(int argc, char **argv) {
    const char *usage = \
        "usage: draw_line <xbegin> <xend> <ybegin> <yend> <color> [-i]\n"
        "  xbegin           x left coord\n"
        "  xend             x right coord\n"
        "  ybegin           y top coord\n"
        "  yend             y bottom coord\n"
        "  color            hex\n"
        "  -i               inverted color";
    if (argc < 6) {
        fputs(usage, fout);
        return;
    }
    size_t x    = strtoul(argv[1], NULL, 0);
    size_t xend = strtoul(argv[2], NULL, 0);
    size_t y    = strtoul(argv[3], NULL, 0);
    size_t yend = strtoul(argv[4], NULL, 0);
    uint32_t color = strtoul(argv[5], NULL, 16);
    int invert = (argc == 7) && !strcmp(argv[6], "-i");
    COVERAGE_ON();
    umka_sys_draw_line(x, xend, y, yend, color, invert);
    COVERAGE_OFF();
}

static void
shell_set_window_caption(int argc, char **argv) {
    const char *usage = \
        "usage: set_window_caption <caption> <encoding>\n"
        "  caption          asciiz string\n"
        "  encoding         1 = cp866, 2 = UTF-16LE, 3 = UTF-8";
    if (argc != 3) {
        fputs(usage, fout);
        return;
    }
    const char *caption = argv[1];
    int encoding = strtoul(argv[2], NULL, 0);
    COVERAGE_ON();
    umka_sys_set_window_caption(caption, encoding);
    COVERAGE_OFF();
}

static void
shell_draw_window(int argc, char **argv) {
    const char *usage = \
        "usage: draw_window <x> <xsize> <y> <ysize> <color> <has_caption>"
            " <client_relative> <fill_workarea> <gradient_fill> <movable>"
            " <style> <caption>\n"
        "  x                x coord\n"
        "  xsize            x size\n"
        "  y                y coord\n"
        "  ysize            y size\n"
        "  color            hex\n"
        "  has_caption      0/1\n"
        "  client_relative  0/1\n"
        "  fill_workarea    0/1\n"
        "  gradient_fill    0/1\n"
        "  movable          0/1\n"
        "  style            1 - draw nothing, 3 - skinned, 4 - skinned fixed\n"
        "  caption          asciiz";
    if (argc != 13) {
        fputs(usage, fout);
        return;
    }
    size_t x     = strtoul(argv[1], NULL, 0);
    size_t xsize = strtoul(argv[2], NULL, 0);
    size_t y     = strtoul(argv[3], NULL, 0);
    size_t ysize = strtoul(argv[4], NULL, 0);
    uint32_t color = strtoul(argv[5], NULL, 16);
    int has_caption = strtoul(argv[6], NULL, 0);
    int client_relative = strtoul(argv[7], NULL, 0);
    int fill_workarea = strtoul(argv[8], NULL, 0);
    int gradient_fill = strtoul(argv[9], NULL, 0);
    int movable = strtoul(argv[10], NULL, 0);
    int style = strtoul(argv[11], NULL, 0);
    const char *caption = argv[12];
    COVERAGE_ON();
    umka_sys_draw_window(x, xsize, y, ysize, color, has_caption,
                         client_relative, fill_workarea, gradient_fill, movable,
                         style, caption);
    COVERAGE_OFF();
}

static void
shell_window_redraw(int argc, char **argv) {
    const char *usage = \
        "usage: window_redraw <1|2>\n"
        "  1                begin\n"
        "  2                end";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    int begin_end = strtoul(argv[1], NULL, 0);
    COVERAGE_ON();
    umka_sys_window_redraw(begin_end);
    COVERAGE_OFF();
}

static void
shell_move_window(int argc, char **argv) {
    const char *usage = \
        "usage: move_window <x> <y> <xsize> <ysize>\n"
        "  x                new x coord\n"
        "  y                new y coord\n"
        "  xsize            x size -1\n"
        "  ysize            y size -1";
    if (argc != 5) {
        fputs(usage, fout);
        return;
    }
    size_t x      = strtoul(argv[1], NULL, 0);
    size_t y      = strtoul(argv[2], NULL, 0);
    ssize_t xsize = strtol(argv[3], NULL, 0);
    ssize_t ysize = strtol(argv[4], NULL, 0);
    COVERAGE_ON();
    umka_sys_move_window(x, y, xsize, ysize);
    COVERAGE_OFF();
}

static void
shell_blit_bitmap(int argc, char **argv) {
    const char *usage = \
        "usage: blit_bitmap <dstx> <dsty> <dstxsize> <dstysize> <srcx> <srcy>"
            " <srcxsize> <srcysize> <operation> <background> <transparent>"
            " <client_relative> <row_length>\n"
        "  dstx             dst rect x offset, window-relative\n"
        "  dsty             dst rect y offset, window-relative\n"
        "  dstxsize         dst rect width\n"
        "  dstysize         dst rect height\n"
        "  srcx             src rect x offset, window-relative\n"
        "  srcy             src rect y offset, window-relative\n"
        "  srcxsize         src rect width\n"
        "  srcysize         src rect height\n"
        "  operation        0 - copy\n"
        "  background       0/1 - blit into background surface\n"
        "  transparent      0/1\n"
        "  client_relative  0/1\n"
        "  row_length       in bytes";
    if (argc != 15) {
        fputs(usage, fout);
        return;
    }
    const char *fname = argv[1];
    FILE *f = fopen(fname, "r");
    if (!f) {
        fprintf(fout, "[!] can't open file '%s': %s\n", fname, strerror(errno));
        return;
    }
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    rewind(f);
    uint8_t *image = (uint8_t*)malloc(fsize);
    fread(image, fsize, 1, f);
    fclose(f);
    size_t dstx     = strtoul(argv[2], NULL, 0);
    size_t dsty     = strtoul(argv[3], NULL, 0);
    size_t dstxsize = strtoul(argv[4], NULL, 0);
    size_t dstysize = strtoul(argv[5], NULL, 0);
    size_t srcx     = strtoul(argv[6], NULL, 0);
    size_t srcy     = strtoul(argv[7], NULL, 0);
    size_t srcxsize = strtoul(argv[8], NULL, 0);
    size_t srcysize = strtoul(argv[9], NULL, 0);
    int operation   = strtoul(argv[10], NULL, 0);
    int background  = strtoul(argv[11], NULL, 0);
    int transparent = strtoul(argv[12], NULL, 0);
    int client_relative = strtoul(argv[13], NULL, 0);
    int row_length = strtoul(argv[14], NULL, 0);
    uint32_t params[] = {dstx, dsty, dstxsize, dstysize, srcx, srcy, srcxsize,
                         srcysize, (uintptr_t)image, row_length};
    COVERAGE_ON();
    umka_sys_blit_bitmap(operation, background, transparent, client_relative,
                         params);
    COVERAGE_OFF();
    free(image);
}

static void
shell_scrot(int argc, char **argv) {
    const char *usage = \
        "usage: scrot <file>\n"
        "  file             path/to/file in png format";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    uint32_t xsize, ysize;
    COVERAGE_ON();
    umka_sys_get_screen_size(&xsize, &ysize);
    COVERAGE_OFF();

    uint32_t *lfb = (uint32_t*)kos_lfb_base;    // assume 32bpp
    for (size_t y = 0; y < ysize; y++) {
        for (size_t x = 0; x < xsize; x++) {
            *lfb++ |= 0xff000000;
        }
    }

    unsigned error = lodepng_encode32_file(argv[1], kos_lfb_base, xsize, ysize);
    if(error) fprintf(fout, "error %u: %s\n", error, lodepng_error_text(error));
}

static void
shell_cd(int argc, char **argv) {
    const char *usage = \
        "usage: cd <path>\n"
        "  path             path/to/dir";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    COVERAGE_ON();
    umka_sys_set_cwd(argv[1]);
    COVERAGE_OFF();
    cur_dir_changed = true;
}

static void
ls_range(f7080s1arg_t *fX0, f70or80_t f70or80) {
    f7080ret_t r;
    size_t bdfe_len = (fX0->encoding == CP866) ? BDFE_LEN_CP866 :
                                                 BDFE_LEN_UNICODE;
    uint32_t requested = fX0->size;
    if (fX0->size > MAX_DIRENTS_TO_READ) {
        fX0->size = MAX_DIRENTS_TO_READ;
    }
    for (; requested; requested -= fX0->size) {
        if (fX0->size > requested) {
            fX0->size = requested;
        }
        COVERAGE_ON();
        umka_sys_lfn(fX0, &r, f70or80);
        COVERAGE_OFF();
        fX0->offset += fX0->size;
        print_f70_status(&r, 1);
        f7080s1info_t *dir = fX0->buf;
        int ok = (r.count <= fX0->size);
        ok &= (dir->cnt == r.count);
        ok &= (r.status == ERROR_SUCCESS && r.count == fX0->size)
              || (r.status == ERROR_END_OF_FILE && r.count < fX0->size);
        assert(ok);
        if (!ok)
            break;
        bdfe_t *bdfe = dir->bdfes;
        for (size_t i = 0; i < dir->cnt; i++) {
            char fattr[KF_ATTR_CNT+1];
            convert_f70_file_attr(bdfe->attr, fattr);
            fprintf(fout, "%s %s\n", fattr, bdfe->name);
            bdfe = (bdfe_t*)((uintptr_t)bdfe + bdfe_len);
        }
        if (r.status == ERROR_END_OF_FILE) {
            break;
        }
    }
}

static void
ls_all(f7080s1arg_t *fX0, f70or80_t f70or80) {
    f7080ret_t r;
    size_t bdfe_len = (fX0->encoding == CP866) ? BDFE_LEN_CP866 :
                                                 BDFE_LEN_UNICODE;
    while (true) {
        COVERAGE_ON();
        umka_sys_lfn(fX0, &r, f70or80);
        COVERAGE_OFF();
        print_f70_status(&r, 1);
        assert((r.status == ERROR_SUCCESS && r.count == fX0->size)
              || (r.status == ERROR_END_OF_FILE && r.count < fX0->size));
        f7080s1info_t *dir = fX0->buf;
        fX0->offset += dir->cnt;
        int ok = (r.count <= fX0->size);
        ok &= (dir->cnt == r.count);
        ok &= (r.status == ERROR_SUCCESS && r.count == fX0->size)
              || (r.status == ERROR_END_OF_FILE && r.count < fX0->size);
        assert(ok);
        if (!ok)
            break;
        fprintf(fout, "total = %"PRIi32"\n", dir->total_cnt);
        bdfe_t *bdfe = dir->bdfes;
        for (size_t i = 0; i < dir->cnt; i++) {
            char fattr[KF_ATTR_CNT+1];
            convert_f70_file_attr(bdfe->attr, fattr);
            fprintf(fout, "%s %s\n", fattr, bdfe->name);
            bdfe = (bdfe_t*)((uintptr_t)bdfe + bdfe_len);
        }
        if (r.status == ERROR_END_OF_FILE) {
            break;
        }
    }
}

static fs_enc_t
parse_encoding(const char *str) {
    fs_enc_t enc;
    if (!strcmp(str, "default")) {
        enc = DEFAULT_ENCODING;
    } else if (!strcmp(str, "cp866")) {
        enc = CP866;
    } else if (!strcmp(str, "utf16")) {
        enc = UTF16;
    } else if (!strcmp(str, "utf8")) {
        enc = UTF8;
    } else {
        enc = INVALID_ENCODING;
    }
    return enc;
}

static void
shell_exec(int argc, char **argv) {
    const char *usage = \
        "usage: exec <file>\n"
        "  file           executable to run";
    if (!argc) {
        fputs(usage, fout);
        return;
    }
    f7080s7arg_t fX0 = {.sf = 7};
    f7080ret_t r;
    int opt = 1;
    fX0.u.f70.zero = 0;
    fX0.u.f70.path = argv[opt++];
    fX0.flags = 0;
    fX0.params = "test";

    COVERAGE_ON();
    umka_sys_lfn(&fX0, &r, F70);
    COVERAGE_OFF();
    if (r.status < 0) {
        r.status = -r.status;
    } else {
        fprintf(fout, "pid: %" PRIu32 "\n", r.status);
        r.status = 0;
    }
    print_f70_status(&r, 1);
}

static void
shell_ls(int argc, char **argv, const char *usage, f70or80_t f70or80) {
    if (!argc) {
        fputs(usage, fout);
        return;
    }
    int opt;
    optind = 1;
    const char *optstring = (f70or80 == F70) ? "f:c:e:" : "f:c:e:p:";
    const char *path = ".";
    uint32_t readdir_enc = DEFAULT_READDIR_ENCODING;
    uint32_t path_enc = DEFAULT_PATH_ENCODING;
    uint32_t from_idx = 0, count = MAX_DIRENTS_TO_READ;
    if (argc > 1 && *argv[optind] != '-') {
        path = argv[optind++];
    }
    while ((opt = getopt(argc, argv, optstring)) != -1) {
        switch (opt) {
        case 'f':
            from_idx = strtoul(optarg, NULL, 0);
            break;
        case 'c':
            count = strtoul(optarg, NULL, 0);
            break;
        case 'e':
            readdir_enc = parse_encoding(optarg);
            break;
        case 'p':
            path_enc = parse_encoding(optarg);
            break;
        default:
            fputs(usage, fout);
            return;
        }
    }

    size_t bdfe_len = (readdir_enc <= CP866) ? BDFE_LEN_CP866 :
                                               BDFE_LEN_UNICODE;
    f7080s1info_t *dir = (f7080s1info_t*)malloc(sizeof(f7080s1info_t) +
                                                bdfe_len * MAX_DIRENTS_TO_READ);
    f7080s1arg_t fX0 = {.sf = 1, .offset = from_idx, .encoding = readdir_enc,
                        .size = count, .buf = dir};
    if (f70or80 == F70) {
        fX0.u.f70.zero = 0;
        fX0.u.f70.path = path;
    } else {
        fX0.u.f80.path_encoding = path_enc;
        fX0.u.f80.path = path;
    }
    if (count != MAX_DIRENTS_TO_READ) {
        ls_range(&fX0, f70or80);
    } else {
        ls_all(&fX0, f70or80);
    }
    free(dir);
    return;
}

static void
shell_ls70(int argc, char **argv) {
    const char *usage = \
        "usage: ls70 [dir] [option]...\n"
        "  -f number        index of the first dir entry to read\n"
        "  -c number        number of dir entries to read\n"
        "  -e encoding      cp866|utf16|utf8\n"
        "                   return directory listing in this encoding";
    shell_ls(argc, argv, usage, F70);
}

static void
shell_ls80(int argc, char **argv) {
    const char *usage = \
        "usage: ls80 [dir] [option]...\n"
        "  -f number        index of the first dir entry to read\n"
        "  -c number        number of dir entries to read\n"
        "  -e encoding      cp866|utf16|utf8\n"
        "                   return directory listing in this encoding\n"
        "  -p encoding      cp866|utf16|utf8\n"
        "                   path to dir is specified in this encoding";
    shell_ls(argc, argv, usage, F80);
}

static void
shell_stat(int argc, char **argv, f70or80_t f70or80) {
    const char *usage = \
        "usage: stat <file>\n"
        "  file             path/to/file";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    f7080s5arg_t fX0 = {.sf = 5, .flags = 0};
    f7080ret_t r;
    bdfe_t file;
    fX0.buf = &file;
    if (f70or80 == F70) {
        fX0.u.f70.zero = 0;
        fX0.u.f70.path = argv[1];
    } else {
        fX0.u.f80.path_encoding = DEFAULT_PATH_ENCODING;
        fX0.u.f80.path = argv[1];
    }
    COVERAGE_ON();
    umka_sys_lfn(&fX0, &r, f70or80);
    COVERAGE_OFF();
    print_f70_status(&r, 0);
    if (r.status != ERROR_SUCCESS)
        return;
    char fattr[KF_ATTR_CNT+1];
    convert_f70_file_attr(file.attr, fattr);
    fprintf(fout, "attr: %s\n", fattr);
    if ((file.attr & KF_FOLDER) == 0) {   // don't show size for dirs
        fprintf(fout, "size: %llu\n", file.size);
    }

#if PRINT_DATE_TIME == 1    // TODO: runtime, argv flag
    time_t time;
    struct tm *t;
    time = kos_time_to_epoch(&file.ctime);
    t = localtime(&time);
    fprintf(fout, "ctime: %4.4i.%2.2i.%2.2i %2.2i:%2.2i:%2.2i\n",
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
    time = kos_time_to_epoch(&file.atime);
    t = localtime(&time);
    fprintf(fout, "atime: %4.4i.%2.2i.%2.2i %2.2i:%2.2i:%2.2i\n",
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
    time = kos_time_to_epoch(&file.mtime);
    t = localtime(&time);
    fprintf(fout, "mtime: %4.4i.%2.2i.%2.2i %2.2i:%2.2i:%2.2i\n",
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
#endif
    return;
}

static void
shell_stat70(int argc, char **argv) {
    shell_stat(argc, argv, F70);
}

static void
shell_stat80(int argc, char **argv) {
    shell_stat(argc, argv, F80);
}

static void
shell_read(int argc, char **argv, f70or80_t f70or80, const char *usage) {
    if (argc < 3) {
        fputs(usage, fout);
        return;
    }
    f7080s0arg_t fX0 = {.sf = 0};
    f7080ret_t r;
    bool dump_bytes = false, dump_hash = false;
    int opt = 1;
    if (f70or80 == F70) {
        fX0.u.f70.zero = 0;
        fX0.u.f70.path = argv[opt++];
    } else {
        fX0.u.f80.path_encoding = DEFAULT_PATH_ENCODING;
        fX0.u.f80.path = argv[opt++];
    }
    if ((opt >= argc) || !parse_uint64(argv[opt++], &fX0.offset))
        return;
    if ((opt >= argc) || !parse_uint32(argv[opt++], &fX0.count))
        return;
    for (; opt < argc; opt++) {
        if (!strcmp(argv[opt], "-b")) {
            dump_bytes = true;
        } else if (!strcmp(argv[opt], "-h")) {
            dump_hash = true;
        } else if (!strcmp(argv[opt], "-e")) {
            if (f70or80 == F70) {
                fprintf(fout, "f70 doesn't accept encoding parameter,"
                        " use f80\n");
                return;
            }
        } else {
            fprintf(fout, "invalid option: '%s'\n", argv[opt]);
            return;
        }
    }
    fX0.buf = (uint8_t*)malloc(fX0.count);

    COVERAGE_ON();
    umka_sys_lfn(&fX0, &r, f70or80);
    COVERAGE_OFF();

    print_f70_status(&r, 1);
    if (r.status == ERROR_SUCCESS || r.status == ERROR_END_OF_FILE) {
        if (dump_bytes)
            print_bytes(fX0.buf, r.count);
        if (dump_hash)
            print_hash(fX0.buf, r.count);
    }

    free(fX0.buf);
    return;
}

static void
shell_read70(int argc, char **argv) {
    const char *usage = \
        "usage: read70 <file> <offset> <length> [-b] [-h]\n"
        "  file             path/to/file\n"
        "  offset           in bytes\n"
        "  length           in bytes\n"
        "  -b               dump bytes in hex\n"
        "  -h               print hash of data read";

    shell_read(argc, argv, F70, usage);
}

static void
shell_read80(int argc, char **argv) {
    const char *usage = \
        "usage: read80 <file> <offset> <length> [-b] [-h]"
            " [-e cp866|utf8|utf16]\n"
        "  file             path/to/file\n"
        "  offset           in bytes\n"
        "  length           in bytes\n"
        "  -b               dump bytes in hex\n"
        "  -h               print hash of data read\n"
        "  -e               encoding";
    shell_read(argc, argv, F80, usage);
}

static void
shell_acpi_preload_table(int argc, char **argv) {
    const char *usage = \
        "usage: acpi_preload_table <file>\n"
        "  file             path/to/local/file.aml";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        fprintf(fout, "[umka] can't open file: %s\n", argv[1]);
        return;
    }
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    rewind(f);
    uint8_t *table = (uint8_t*)malloc(fsize);
    fread(table, fsize, 1, f);
    fclose(f);
    fprintf(fout, "table #%zu\n", kos_acpi_ssdt_cnt);
    kos_acpi_ssdt_base[kos_acpi_ssdt_cnt] = table;
    kos_acpi_ssdt_size[kos_acpi_ssdt_cnt] = fsize;
    kos_acpi_ssdt_cnt++;
}

static void
shell_pci_set_path(int argc, char **argv) {
    const char *usage = \
        "usage: pci_set_path <path>\n"
        "  path           where aaaa:bb:cc.d dirs are";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    strcpy(pci_path, argv[1]);
}

static void
shell_pci_get_path(int argc, char **argv) {
    (void)argv;
    const char *usage = \
        "usage: pci_get_path";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    fprintf(fout, "pci path: %s\n", pci_path);
}

static void
shell_bg_set_size(int argc, char **argv) {
    const char *usage = \
        "usage: bg_set_size <xsize> <ysize>\n"
        "  xsize          in pixels\n"
        "  ysize          in pixels";
    if (argc != 3) {
        fputs(usage, fout);
        return;
    }
    uint32_t xsize = strtoul(argv[1], NULL, 0);
    uint32_t ysize = strtoul(argv[2], NULL, 0);
    umka_sys_bg_set_size(xsize, ysize);
}

static void
shell_bg_put_pixel(int argc, char **argv) {
    const char *usage = \
        "usage: bg_put_pixel <offset> <color>\n"
        "  offset         in bytes, (x+y*xsize)*3\n"
        "  color          in hex";
    if (argc != 3) {
        fputs(usage, fout);
        return;
    }
    size_t offset = strtoul(argv[1], NULL, 0);
    uint32_t color = strtoul(argv[2], NULL, 0);
    umka_sys_bg_put_pixel(offset, color);
}

static void
shell_bg_redraw(int argc, char **argv) {
    (void)argv;
    const char *usage = \
        "usage: bg_redraw";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    umka_sys_bg_redraw();
}

static void
shell_bg_set_mode(int argc, char **argv) {
    const char *usage = \
        "usage: bg_set_mode <mode>\n"
        "  mode           1 = tile, 2 = stretch";
    if (argc != 3) {
        fputs(usage, fout);
        return;
    }
    uint32_t mode = strtoul(argv[1], NULL, 0);
    umka_sys_bg_set_mode(mode);
}

static void
shell_bg_put_img(int argc, char **argv) {
    const char *usage = \
        "usage: bg_put_img <image> <offset>\n"
        "  image          file\n"
        "  offset         in bytes, (x+y*xsize)*3\n";
    if (argc != 4) {
        fputs(usage, fout);
        return;
    }
    FILE *f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    rewind(f);
    uint8_t *image = (uint8_t*)malloc(fsize);
    fread(image, fsize, 1, f);
    fclose(f);
    size_t offset = strtoul(argv[2], NULL, 0);
    umka_sys_bg_put_img(image, offset, fsize);
}

static void
shell_bg_map(int argc, char **argv) {
    (void)argv;
    const char *usage = \
        "usage: bg_map";
    if (argc != 1) {
        fputs(usage, fout);
        return;
    }
    void *addr = umka_sys_bg_map();
    fprintf(fout, "%p\n", addr);
}

static void
shell_bg_unmap(int argc, char **argv) {
    const char *usage = \
        "usage: bg_unmap <addr>\n"
        "  addr           return value of bg_map";
    if (argc != 2) {
        fputs(usage, fout);
        return;
    }
    void *addr = (void*)strtoul(argv[1], NULL, 0);
    uint32_t status = umka_sys_bg_unmap(addr);
    fprintf(fout, "status = %d\n", status);
}

static void shell_help(int argc, char **argv);

func_table_t shell_cmds[] = {
    { "umka_init",               shell_umka_init },
    { "umka_set_boot_params",    shell_umka_set_boot_params },
    { "acpi_preload_table",      shell_acpi_preload_table },
    { "bg_map",                  shell_bg_map },
    { "bg_put_img",              shell_bg_put_img },
    { "bg_put_pixel",            shell_bg_put_pixel },
    { "bg_redraw",               shell_bg_redraw },
    { "bg_set_mode",             shell_bg_set_mode },
    { "bg_set_size",             shell_bg_set_size },
    { "bg_unmap",                shell_bg_unmap },
    { "blit_bitmap",             shell_blit_bitmap },
    { "button",                  shell_button },
    { "cd",                      shell_cd },
    { "set",                     shell_set },
    { "disk_add",                shell_disk_add },
    { "disk_del",                shell_disk_del },
    { "display_number",          shell_display_number },
    { "draw_line",               shell_draw_line },
    { "draw_rect",               shell_draw_rect },
    { "draw_window",             shell_draw_window },
    { "dump_appdata",            shell_dump_appdata },
    { "dump_taskdata",           shell_dump_taskdata },
    { "dump_win_pos",            shell_dump_win_pos },
    { "dump_win_stack",          shell_dump_win_stack },
    { "dump_win_map",            shell_dump_win_map },
    { "exec",                    shell_exec },
    { "get_font_size",           shell_get_font_size },
    { "get_font_smoothing",      shell_get_font_smoothing },
    { "get_screen_area",         shell_get_screen_area },
    { "get_screen_size",         shell_get_screen_size },
    { "get_skin_height",         shell_get_skin_height },
    { "get_skin_margins",        shell_get_skin_margins },
    { "get_window_colors",       shell_get_window_colors },
    { "help",                    shell_help },
    { "i40",                     shell_i40 },
    { "ls70",                    shell_ls70 },
    { "ls80",                    shell_ls80 },
    { "move_window",             shell_move_window },
    { "mouse_move",              shell_mouse_move },
    { "pci_get_path",            shell_pci_get_path },
    { "pci_set_path",            shell_pci_set_path },
    { "process_info",            shell_process_info },
    { "put_image",               shell_put_image },
    { "put_image_palette",       shell_put_image_palette },
    { "pwd",                     shell_pwd },
    { "ramdisk_init",            shell_ramdisk_init },
    { "read70",                  shell_read70 },
    { "read80",                  shell_read80 },
    { "scrot",                   shell_scrot },
    { "set_button_style",        shell_set_button_style },
    { "set_cwd",                 shell_cd },
    { "set_font_size",           shell_set_font_size },
    { "set_font_smoothing",      shell_set_font_smoothing },
    { "set_pixel",               shell_set_pixel },
    { "set_screen_area",         shell_set_screen_area },
    { "set_skin",                shell_set_skin },
    { "set_window_caption",      shell_set_window_caption },
    { "set_window_colors",       shell_set_window_colors },
    { "stat70",                  shell_stat70 },
    { "stat80",                  shell_stat80 },
    { "window_redraw",           shell_window_redraw },
    { "write_text",              shell_write_text },
    { "switch_to_thread",        shell_switch_to_thread },
    { "new_sys_thread",          shell_new_sys_thread },
};

static void
shell_help(int argc, char **argv) {
    const char *usage = \
        "usage: help [command]\n"
        "  command        help on this command usage";
    switch (argc) {
    case 1:
        fputs(usage, fout);
        fputs("\navailable commands:\n", fout);
        for (func_table_t *ft = shell_cmds;
             ft < shell_cmds + sizeof(shell_cmds) / sizeof(*shell_cmds);
             ft++) {
            fprintf(fout, "  %s\n", ft->name);
        }
        break;
    case 2: {
        const char *cmd_name = argv[1];
        size_t i;
        for (i = 0; i < sizeof(shell_cmds) / sizeof(*shell_cmds); i++) {
            if (!strcmp(shell_cmds[i].name, cmd_name)) {
                shell_cmds[i].func(0, NULL);
                return;
            }
        }
        fprintf(fout, "no such command: %s\n", cmd_name);
        break;
    }
    default:
        fputs(usage, fout);
        return;
    }
}

void *
run_test(FILE *in, FILE *out) {
    fin = in;
    fout = out;
    int is_tty = 0; // isatty(fileno(fin));
    char **argv = (char**)malloc(sizeof(char*) * (MAX_COMMAND_ARGS + 1));
    while(next_line(is_tty)) {
        if (cmd_buf[0] == '#' || cmd_buf[0] == '\n' || cmd_buf[0] == '\0' ||
            cmd_buf[0] == '\r') {
            fprintf(fout, "%s", cmd_buf);
            continue;
        }
        if (cmd_buf[0] == 'X') break;
        if (!is_tty) {
            prompt();
            fprintf(fout, "%s", cmd_buf);
            fflush(fout);
        }
        int argc = split_args(cmd_buf, argv);
        func_table_t *ft;
        for (ft = shell_cmds;
             ft < shell_cmds + sizeof(shell_cmds) / sizeof(*shell_cmds);
             ft++) {
            if (!strcmp(argv[0], ft->name)) {
                break;
            }
        }
        if (ft->name) {
            ft->func(argc, argv);
        } else {
            fprintf(fout, "unknown command: %s\n", argv[0]);
        }
    }
    free(argv);

    return NULL;
}
