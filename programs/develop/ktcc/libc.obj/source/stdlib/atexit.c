/*
* SPDX-License-Identifier: GPL-2.0-only
* Copyright (C) 2026 KolibriOS team
*/

#include <stdlib.h>

struct atexit_n {
    struct atexit_n* last;
    void (*func)(void);
};

static struct atexit_n* __last_atexit_node = NULL;

int atexit(void (*func)(void))
{
    struct atexit_n* n = malloc(sizeof(struct atexit_n));

    if (n == NULL) {
        return 1;
    }

    n->last = __last_atexit_node;
    n->func = func;

    __last_atexit_node = n;

    return 0;
}

void __run_atexit()
{
    struct atexit_n* n = __last_atexit_node;
    while (n != NULL) {
        n->func();
        struct atexit_n* to_free = n;
        n = n->last;
        free(to_free);
    }
}
