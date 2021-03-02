#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct List_s {
    char *this;
    struct List_s *next;
} List;

int main() {
    List *root;
    for (List **pitem = &root;; pitem = &(*pitem)->next) {
        size_t n = 1024;
        *pitem = calloc(1, sizeof(List));
        List *item = *pitem;
        item->this = calloc(1, n);
        fgets(item->this, n, stdin);
        if (item->this[0] == '\n') {
            free(*pitem);
            *pitem = NULL;
            break;
        } else {
            item->this[strlen(item->this) - 1] = '\0';
        }
    }

    for (List *item = root; item; item = item->next) {
        char asm_name[255];
        sprintf(asm_name, "%s.asm", item->this);
        FILE *out = fopen(asm_name, "wb");

        fprintf(out, "format ELF\n");
        fprintf(out, "include \"__lib__.inc\"\n");
        fprintf(out, "fun      equ __func@%s\n", item->this);
        fprintf(out, "fun_str  equ '%s'\n", item->this);
        fprintf(out, "section '.text'\n");
        fprintf(out, "fun_name db fun_str, 0\n");
        fprintf(out, "section '.data'\n");
        fprintf(out, "extrn lib_name\n");
        fprintf(out, "public fun as fun_str\n");
        fprintf(out, "fun dd fun_name\n");
        fprintf(out, "lib dd lib_name\n");

        fclose(out);
    }
}
