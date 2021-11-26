#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("usage: lfbviewx <pid> <address> <width> <height>\n");
        exit(1);
    }
    int depth = 32;
    int umka_pid = strtol(argv[1], NULL, 0);
    uintptr_t umka_lfb_addr = strtol(argv[2], NULL, 0);
    size_t lfb_width = strtoul(argv[3], NULL, 0);
    size_t lfb_height = strtoul(argv[4], NULL, 0);

    Display *display = XOpenDisplay(NULL);
    int screen_num = XDefaultScreen(display);
    Window root = XDefaultRootWindow(display);

    XVisualInfo vis_info;
    if(!XMatchVisualInfo(display, screen_num, depth, TrueColor, &vis_info)) {
        fprintf(stderr, "ERR: %d-bit depth is not supported\n", depth);
        exit(1);
    }

    Visual *visual = vis_info.visual;
    XSetWindowAttributes win_attr = {
        .colormap = XCreateColormap(display, root, visual, AllocNone),
        .background_pixel = 0,
        .border_pixel = 0};
    unsigned long win_mask = CWBackPixel | CWColormap | CWBorderPixel;
    Window window = XCreateWindow(display, root, 0, 0, lfb_width, lfb_height, 0, depth, InputOutput, visual, win_mask, &win_attr);
    GC gc = XCreateGC(display, window, 0, 0);

    uint32_t *lfb = (uint32_t*)malloc(lfb_width*lfb_height*sizeof(uint32_t));
    XImage *image = XCreateImage(display, visual, depth, ZPixmap, 0, (char*)lfb, lfb_width, lfb_height, 32, 0);

    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XStoreName(display, window, "KolibriOS LFB Viewer for X");
    XMapWindow(display, window);


    struct iovec remote = {.iov_base = (void*)umka_lfb_addr,
                           .iov_len = lfb_width*lfb_height*4};
    struct iovec local = {.iov_base = lfb,
                          .iov_len = lfb_width*lfb_height*4};
/*
    XEvent event;
    while (true) {
        XNextEvent(display, &event);
        if (event.type == Expose) {
            process_vm_readv(umka_pid, &local, 1, &remote, 1, 0);
            XPutImage(display, window, gc, image, 0, 0, 0, 0, lfb_width, lfb_height);
        } else if (event.type == KeyPress) {
            int keysym = XkbKeycodeToKeysym(display, event. xkey.keycode, 0, 0);

            if (keysym == XK_Escape) break;
            switch (keysym) {
            case XK_Left:   {break;}
            }
        }
    }
*/
    XEvent event;
    while (true) {
        while (XCheckMaskEvent(display, (long)-1, &event)) { /* skip */ }
        process_vm_readv(umka_pid, &local, 1, &remote, 1, 0);
        XPutImage(display, window, gc, image, 0, 0, 0, 0, lfb_width, lfb_height);
        sleep(1);
    }

    XCloseDisplay(display);
    return 0;
}
