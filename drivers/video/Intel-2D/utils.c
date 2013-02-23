
#include <stdint.h>
#include <stdio.h>


static inline void native_cpuid(unsigned int *eax, unsigned int *ebx,
                unsigned int *ecx, unsigned int *edx)
{
    /* ecx is often an input as well as an output. */
    asm volatile("cpuid"
        : "=a" (*eax),
          "=b" (*ebx),
          "=c" (*ecx),
          "=d" (*edx)
        : "0" (*eax), "2" (*ecx)
        : "memory");
}

/* Some CPUID calls want 'count' to be placed in ecx */
static inline void cpuid_count(unsigned int op, int count,
                               unsigned int *eax, unsigned int *ebx,
                               unsigned int *ecx, unsigned int *edx)
{
    *eax = op;
    *ecx = count;
    native_cpuid(eax, ebx, ecx, edx);
}


enum _cache_type {
        CACHE_TYPE_NULL = 0,
        CACHE_TYPE_DATA = 1,
        CACHE_TYPE_INST = 2,
        CACHE_TYPE_UNIFIED = 3
};


union _cpuid4_leaf_eax {
        struct {
                enum _cache_type        type:5;
                unsigned int            level:3;
                unsigned int            is_self_initializing:1;
                unsigned int            is_fully_associative:1;
                unsigned int            reserved:4;
                unsigned int            num_threads_sharing:12;
                unsigned int            num_cores_on_die:6;
        } split;
        uint32_t full;
};

union _cpuid4_leaf_ebx {
        struct {
                unsigned int            coherency_line_size:12;
                unsigned int            physical_line_partition:10;
                unsigned int            ways_of_associativity:10;
        } split;
        uint32_t full;
};

union _cpuid4_leaf_ecx {
        struct {
                unsigned int            number_of_sets:32;
        } split;
        uint32_t full;
};

struct _cpuid4_info_regs {
        union _cpuid4_leaf_eax eax;
        union _cpuid4_leaf_ebx ebx;
        union _cpuid4_leaf_ecx ecx;
        unsigned long size;
};

static int
cpuid4_cache_lookup_regs(int index,
                   struct _cpuid4_info_regs *this_leaf)
{
    union _cpuid4_leaf_eax  eax;
    union _cpuid4_leaf_ebx  ebx;
    union _cpuid4_leaf_ecx  ecx;
    unsigned                edx;

    cpuid_count(4, index, &eax.full, &ebx.full, &ecx.full, &edx);

    if (eax.split.type == CACHE_TYPE_NULL)
        return -1; /* better error ? */

    this_leaf->eax = eax;
    this_leaf->ebx = ebx;
    this_leaf->ecx = ecx;
    this_leaf->size = (ecx.split.number_of_sets  + 1) *
              (ebx.split.coherency_line_size     + 1) *
              (ebx.split.physical_line_partition + 1) *
              (ebx.split.ways_of_associativity   + 1);
    return 0;
}

static int find_num_cache_leaves()
{
    unsigned int        eax, ebx, ecx, edx, op;
    union _cpuid4_leaf_eax  cache_eax;
    int             i = -1;

    do {
        ++i;
        /* Do cpuid(op) loop to find out num_cache_leaves */
        cpuid_count(4, i, &eax, &ebx, &ecx, &edx);
        cache_eax.full = eax;
    } while (cache_eax.split.type != CACHE_TYPE_NULL);
    return i;
};

unsigned int cpu_cache_size()
{
    unsigned int new_l1d = 0, new_l1i = 0; /* Cache sizes from cpuid(4)  */
    unsigned int new_l2 = 0, new_l3 = 0, i; /* Cache sizes from cpuid(4) */
    unsigned int num_cache_leaves;

    num_cache_leaves = find_num_cache_leaves();

    for (i = 0; i < num_cache_leaves; i++)
    {
        struct _cpuid4_info_regs this_leaf;
        int retval;

        retval = cpuid4_cache_lookup_regs(i, &this_leaf);
        if (retval >= 0) {
            switch (this_leaf.eax.split.level)
            {
                case 1:
                    if (this_leaf.eax.split.type == CACHE_TYPE_DATA)
                        new_l1d = this_leaf.size;
                    else if (this_leaf.eax.split.type == CACHE_TYPE_INST)
                        new_l1i = this_leaf.size;
                    break;
                case 2:
                    new_l2 = this_leaf.size;
                    break;
                case 3:
                    new_l3 = this_leaf.size;
                    break;
                default:
                    break;
            }
        }
    }
    printf("l2 cache %d l3 cache %d\n", new_l2, new_l3);

    return new_l3 != 0 ? new_l3 : new_l2;
};
