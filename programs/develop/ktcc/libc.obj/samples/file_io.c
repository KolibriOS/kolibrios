#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define READ_MAX 255

char* workdir = NULL;

struct generated_files_list {
    char name[FILENAME_MAX];
    struct generated_files_list* next;
};

struct generated_files_list generated_files_list_root;
struct generated_files_list* generated_files_list_top = &generated_files_list_root;

void generated_files_list_add(const char* name)
{
    struct generated_files_list* i = malloc(sizeof(struct generated_files_list));

    strncpy(i->name, name, sizeof(i->name));

    generated_files_list_top->next = i;
    generated_files_list_top = i;
}

void remove_generated_files()
{
    for (struct generated_files_list* i = generated_files_list_root.next; i; i = i->next) {
        remove(i->name);
    }
}

int base_io_test()
{
    static char test_str1[] = "123454567890abcdefghijklmnopqrstufvwxyz";
    static char test_str2[READ_MAX];

    char filename[FILENAME_MAX] = ".";

    if (workdir) {
        strncpy(filename, workdir, sizeof(filename));
    }

    strncat(filename, "/testfile.txt", sizeof(filename));

    int i = 0;
    FILE* f;

    // write to file
    debug_printf("Write file...\n");
    f = fopen(filename, "w");
    generated_files_list_add(filename);

    while (test_str1[i] != 'a') {
        fputc(test_str1[i], f);
        i++;
    }
    fclose(f);

    // append to file
    debug_printf("Apend file...\n");
    f = fopen(filename, "a");
    fputs(test_str1 + i, f);
    char null_term = '\0';
    fwrite(&null_term, sizeof(char), 1, f);
    printf("Error: %s\n", strerror(errno));
    fclose(f);

    // copy from testfile.txt to copyfile.txt
    debug_printf("Read file...\n");
    f = fopen(filename, "r");
    i = 0;
    while ((test_str2[i] = fgetc(f)) != EOF && i < READ_MAX) {
        fputc(test_str2[i], stdout);
        i++;
    }
    printf("\n%s\n", test_str1);
    if (!strcmp(test_str2, test_str1)) {
        puts("TEST: OK!");
    } else {
        puts("TEST: FAIL!");
    }
    fclose(f);

    return 0;
}

int big_file_test()
{
    char FILE_NAME[FILENAME_MAX] = "/hd1/1";
    static const uint64_t FILE_SIZE = (5ULL * 1024 * 1024 * 1024); // 5Gb
    static const size_t BUFFER_SIZE = (1024 * 1024);

    if (workdir) {
        strncpy(FILE_NAME, workdir, sizeof(FILE_NAME));
    }

    strncat(FILE_NAME, "/large_test_file.bin", sizeof(FILE_NAME));

    FILE* file = NULL;
    uint8_t* buffer = NULL;
    uint64_t bytes_written = 0;

    char test_string[] = "LIBC_LARGE_FILE_TEST_PASSED";
    char read_string[sizeof(test_string)] = { 0 };

    printf("=== Starting libc Large File Test (>4 GB) ===\n");
    printf("Target file size: %.2f GiB (%llu bytes)\n", (double)FILE_SIZE / (1024 * 1024 * 1024), (unsigned long long)FILE_SIZE);

    // 1. Allocate memory for the write buffer
    buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        fprintf(stderr, "Error allocating memory for buffer: %s (errno: %d)\n", strerror(errno), errno);
        return 1;
    }
    memset(buffer, 0xAA, BUFFER_SIZE); // Fill buffer with a dummy pattern

    // 2. Create and fill the file
    file = fopen(FILE_NAME, "wb+");
    generated_files_list_add(FILE_NAME);
    if (!file) {
        fprintf(stderr, "Error creating file: %s (errno: %d)\n", strerror(errno), errno);
        free(buffer);
        return 1;
    }

    printf("Writing %d GiB of data (this may take some time)... \n", FILE_SIZE / 1024 / 1024 / 1024);
    fflush(stdout);

    while (bytes_written < FILE_SIZE) {
        size_t count = fwrite(buffer, 1, BUFFER_SIZE, file);
        if (count == 0) {
            fprintf(stderr, "\nError during block write: %s (errno: %d)\n", strerror(errno), errno);
            fclose(file);
            free(buffer);
            return 1;
        }
        bytes_written += count;
        printf("\r %llu/%llu", bytes_written, FILE_SIZE);
    }

    printf("\nDone.\n");
    free(buffer); // Buffer is no longer needed

    // 3. Testing 64-bit positioning
    printf("Seeking pointer to a position >4 GB... ");

    // Fallback for POSIX/C11 environments without fseeko
    fpos_t pos;
    pos = FILE_SIZE - 100;
    if (fsetpos(file, &pos) != 0) {
        fprintf(stderr, "Error executing fsetpos: %s (errno: %d)\n", strerror(errno), errno);
        fclose(file);
        return 1;
    }
    printf("Done.\n");

    // 4. Write a verification string near the end of the large file
    printf("Writing verification tag... ");
    if (fwrite(test_string, 1, strlen(test_string), file) != strlen(test_string)) {
        fprintf(stderr, "Error writing tag: %s (errno: %d)\n", strerror(errno), errno);
        fclose(file);
        return 1;
    }
    printf("Done.\n");

    // 5. Reset buffers and read back the verification tag
    printf("Flushing streams and reading the tag back... ");
    fflush(file);

    if (fsetpos(file, &pos) != 0) {
        fprintf(stderr, "Error executing fsetpos before read: %s (errno: %d)\n", strerror(errno), errno);
        fclose(file);
        return 1;
    }

    if (fread(read_string, 1, strlen(test_string), file) != strlen(test_string)) {
        fprintf(stderr, "Error reading tag: %s (errno: %d)\n", strerror(errno), errno);
        fclose(file);
        return 1;
    }

    fclose(file);

    printf("\nRead string: %s\n", read_string);
    if (strcmp(test_string, read_string) == 0) {
        printf("=== TEST SUCCESSFUL! libc handles files >4 GB correctly ===\n");
        return 0;
    }

    printf("=== TEST FAILED! Data corruption or offset mismatch ===\n");
    return 1;
}

typedef int (*func_t)();

const static func_t tests_list[] = {
    base_io_test,
    big_file_test
};

int main(int argc, char** argv)
{
    if (argc > 1) {
        workdir = argv[1];
    }

    atexit(remove_generated_files);

    for (size_t i = 0; i < sizeof(tests_list) / sizeof(*tests_list); i++) {
        if (tests_list[i]()) {
            printf("Fail test #%d\n", i + 1);
            return 1;
        }
    }

    return 0;
}
