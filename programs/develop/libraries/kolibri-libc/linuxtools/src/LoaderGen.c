#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int main(int argc, char** argv) {
    if(argc!=3){
        printf("Usage: LoadGen <symbols.txt> <loader_dir>\n");
        return 0;
    }
    argv[2][strlen(argv[1])-2]='\0';
    FILE* symbols_txt = fopen(argv[1], "r");
    if(!symbols_txt){
        fprintf(stderr, "File '%s' not found!\n", argv[1]);
            return -1;
    }
    char line[256];
    while(fgets(line, 256, symbols_txt)) {
        if(line[0]!='!'){
            if(line[strlen(line)-1]=='\n'){
                line[strlen(line)-1]='\0';
            }
            char asm_name[PATH_MAX];
            sprintf(asm_name, "%s/%s.asm", argv[2], line);
            FILE *out = fopen(asm_name, "wb");
            if(!out){
                fprintf(stderr, "Error! File '%s' not created!\n", asm_name);
                return -1;
            }else{
                printf("File '%s' created successfully!\n", asm_name);
            }

            fprintf(out, "format ELF\n");
            fprintf(out, "include \"__lib__.inc\"\n");
            fprintf(out, "fun      equ __func@%s\n", line);
            fprintf(out, "fun_str  equ '%s'\n", line);
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
}
