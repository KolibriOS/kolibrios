#include "kosnet/network.h"
#include "kosnet/dlfcn.h"

int (*inet_addr)(const char* hostname) __attribute__ ((stdcall));
char* (*inet_ntoa)(int ip_addr) __attribute__ ((stdcall));
int (*getaddrinfo)(const char* hostname, const char* servname, const struct addrinfo* hints, struct addrinfo** res) __attribute__ ((stdcall));
void (*freeaddrinfo)(struct addrinfo* ai) __attribute__ ((stdcall));
 
int load_network_obj() {
    void *network_lib = dlopen("/sys/lib/network.obj", RTLD_GLOBAL);
    if (network_lib == NULL) {
        return -1;
    }
    inet_addr = dlsym(network_lib, "inet_addr");
    inet_ntoa = dlsym(network_lib, "inet_ntoa");
    getaddrinfo = dlsym(network_lib, "getaddrinfo");
    freeaddrinfo = dlsym(network_lib, "freeaddrinfo");
    dlclose(network_lib);
    return 0; 
}
 