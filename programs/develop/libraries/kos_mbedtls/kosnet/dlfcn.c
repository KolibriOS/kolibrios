#include <stdlib.h>
#include <string.h>

#include "kosnet/dlfcn.h"
#include "kosnet/kos32sys1.h"

typedef struct {
    char *name;
    void *ptr;
} KosExp;

typedef struct {
    void **importNames;
    char * libraryName;
} KosImp;

static int __attribute__ ((stdcall)) dll_Load(KosImp *importTableEntry);

static const char *__error;

static int __attribute__ ((stdcall)) dll_Load(KosImp *importTableEntry) {
    for (; importTableEntry->importNames; importTableEntry++) {
        char libPath[256] = "/sys/lib/";
        KosExp *exports = NULL;
        void **libImports = importTableEntry->importNames;
        
        strcat(libPath, importTableEntry->libraryName);
        if (!(exports = dlopen(libPath, 0))) { return 1; }
        for (; *libImports; libImports++) {
            if (!(*libImports = dlsym(exports, *libImports))) { return 1; }
        }
    }
    return 0;
}

// https://pubs.opengroup.org/onlinepubs/007908799/xsh/dlopen.html
// Current implementation fully ignores "mode" parameter
void *dlopen(const char *name, int mode) {
    KosExp *exports = NULL;

    // load library using syscall
    asm volatile ("int $0x40":"=a"(exports):"a"(68), "b"(19), "c"(name));
    if (!exports) {
        char libPath[256] = "/sys/lib/";

        strcat(libPath, name);
        asm volatile ("int $0x40":"=a"(exports):"a"(68), "b"(19), "c"(libPath));
        if (!exports) {
            __error = "Library not found in \"/sys/lib/\" nor current folder";
            return NULL;
        }
    }
    // call anything starting with "lib_"
    for (KosExp *export = exports; export->name; export++) {
        if (!memcmp(export->name, "lib_", 4)) {
            asm volatile (
                "call *%4" ::
                "a"(0),
                "b"(0),
                "c"(0),
                "d"(dll_Load),
                "r"(export->ptr));
            // was asm volatile ("call *%4" ::"a"(sysmalloc),"b"(sysfree),"c"(sysrealloc),"d"(dll_Load),"r"(export->ptr));
        }
    }
    return exports;
}

// https://pubs.opengroup.org/onlinepubs/007908799/xsh/dlsym.html
void *dlsym(void *handle, const char *name) {
    KosExp *exp = handle;

    for (; exp->name; exp++) {
        if (!strcmp(exp->name, name)) {
            return exp->ptr;
        }
    }
    __error = "Symbol not found";
    return NULL;
}

// https://pubs.opengroup.org/onlinepubs/007908799/xsh/dlclose.html
int dlclose(void *handle) {
    return 0;
}

// https://pubs.opengroup.org/onlinepubs/007908799/xsh/dlerror.html
char *dlerror(void) {
    char *ret = __error ? strdup(__error) : NULL;
    __error = NULL;
    return ret;
}
