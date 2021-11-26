#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define MAX_COVERED_CODE_SIZE (256*1024)

typedef struct {
    uint64_t to_cnt, from_cnt;
} branch;

branch branches[MAX_COVERED_CODE_SIZE];

uint32_t coverage_offset, coverage_begin, coverage_end;

void read_coverage_file(const char *fname) {
    FILE *f = fopen(fname, "r");
    fseeko(f, 0, SEEK_END);
    off_t fsize = ftello(f);
    fseeko(f, 0, SEEK_SET);
    fread(&coverage_begin, sizeof(uint32_t), 1, f);
    fread(&coverage_end, sizeof(uint32_t), 1, f);
    size_t branch_cnt = (fsize-4*2)/(2*4);
    for (size_t i = 0; i < branch_cnt; i++) {
        uint32_t from, to;
        fread(&from, sizeof(uint32_t), 1, f);
        fread(&to, sizeof(uint32_t), 1, f);
        if (from >= coverage_begin && from < coverage_end) {
            from = from - coverage_begin + coverage_offset;
            branches[from].from_cnt++;
        }
        if (to >= coverage_begin && to < coverage_end) {
            to = to - coverage_begin + coverage_offset;
            branches[to].to_cnt++;
        }
    }
    fclose(f);
}

size_t count_line_bytes(const char *s) {
    size_t cnt = 0;
    for (size_t i = 10; i <= 58; i += 3) {
        if (s[i] == ' ') break;
        cnt++;
    }
    return cnt;
}

size_t count_block_bytes(FILE *f) {
    char tmp[1024];
    size_t cnt = 0;
    if (fgets(tmp, 1024, f) && strspn(tmp, "0123456789ABCDEF") == 8) {
        cnt = count_line_bytes(tmp);
        while (fgets(tmp, 1024, f) && tmp[0] == ' ' && tmp[10] != ' ') {
            cnt += count_line_bytes(tmp);
        }
    }
    return cnt;
}

int is_cond_jump(const char *s) {
    s += strspn(s, " \t");
    int found = !strncmp(s, "jo", 2)
                || !strncmp(s, "jno", 3)
                || !strncmp(s, "js", 2)
                || !strncmp(s, "jns", 3)
                || !strncmp(s, "je", 2)
                || !strncmp(s, "jne", 3)
                || !strncmp(s, "jz", 2)
                || !strncmp(s, "jnz", 3)
                || !strncmp(s, "jb", 2)
                || !strncmp(s, "jnb", 3)
                || !strncmp(s, "jc", 2)
                || !strncmp(s, "jnc", 3)
                || !strncmp(s, "jae", 3)
                || !strncmp(s, "jnae", 4)
                || !strncmp(s, "jbe", 3)
                || !strncmp(s, "jna", 3)
                || !strncmp(s, "ja", 2)
                || !strncmp(s, "jnbe", 4)
                || !strncmp(s, "jl", 2)
                || !strncmp(s, "jnge", 4)
                || !strncmp(s, "jge", 3)
                || !strncmp(s, "jnl", 3)
                || !strncmp(s, "jle", 3)
                || !strncmp(s, "jng", 3)
                || !strncmp(s, "jg",2)
                || !strncmp(s, "jnle", 4)
                || !strncmp(s, "jp", 2)
                || !strncmp(s, "jpe", 3)
                || !strncmp(s, "jnp", 3)
                || !strncmp(s, "jpo", 3)
                || !strncmp(s, "jcxz", 4)
                || !strncmp(s, "jecxz", 5);
    return found;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "usage: covpreproc <listing file> <coverage_begin offset> <coverage files ...>\n");
        exit(1);
    }
    sscanf(argv[2], "%" SCNx32, &coverage_offset);

    for (int i = 3; i < argc; i++) {
        read_coverage_file(argv[i]);
    }

    FILE *f = fopen(argv[1], "r");
    char tmp[1024];
    uint64_t cur = 0, taken, not_taken;
    while (1) {
        off_t fpos_before = ftello(f);
        if (!fgets(tmp, 1024, f)) {
            break;
        }
        off_t fpos_after = ftello(f);
        if (strspn(tmp, "0123456789ABCDEF") == 8) {
            int is_cond = is_cond_jump(tmp + 64);
            fseeko(f, fpos_before, SEEK_SET);
            size_t inst_len = count_block_bytes(f);
            fseeko(f, fpos_after, SEEK_SET);
            unsigned long pos = strtoul(tmp, NULL, 16);
            size_t total_to = 0, total_from = 0;
            for (size_t i = 0; i < inst_len; i++) {
                if (pos + i < coverage_end - coverage_begin + coverage_offset) {
                    total_to += branches[pos + i].to_cnt;
                    total_from += branches[pos + i].from_cnt;
                }
            }
            cur += total_to;
            if (is_cond) {
                taken = total_from;
                not_taken = cur - taken;
                if (taken && not_taken) {
                    putchar(' ');
                } else {
                    putchar('-');
                }
            } else {
                putchar(' ');
            }
            if (cur) {
                putchar(' ');
            } else {
                putchar('-');
            }
            if (is_cond) {
                int spaces = 19 - printf("%10" PRIu64 "/%" PRIu64, taken, not_taken);
                while (spaces-- > 0)
                    putchar(' ');
            } else {
                printf("                   ");
            }
            printf(" %10" PRIu64, cur);
            cur -= total_from;
        } else {
            printf("                                ");
        }
        printf(" : %s", tmp + 64);
    }
    fclose(f);

    return 0;
}
