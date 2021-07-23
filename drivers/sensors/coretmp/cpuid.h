/*
 * cpuid.h --
 *      contains the data structures required for CPUID 
 *      implementation.
 */

#define CPUID_VENDOR_LENGTH     3               /* 3 GPRs hold vendor ID */
#define CPUID_VENDOR_STR_LENGTH (CPUID_VENDOR_LENGTH * sizeof(uint32_t) + 1)
#define CPUID_BRAND_LENGTH      12              /* 12 GPRs hold vendor ID */
#define CPUID_BRAND_STR_LENGTH  (CPUID_BRAND_LENGTH * sizeof(uint32_t) + 1)

extern struct cpu_ident cpu_id;
/*
static inline void __cpuid(unsigned int *eax, unsigned int *ebx,
                                unsigned int *ecx, unsigned int *edx)
{
        /* ecx is often an input as well as an output. */
        asm volatile("\t"
      	    "push %%ebx; cpuid; mov %%ebx, %%edi; pop %%ebx"
            : "=a" (*eax),
              "=D" (*ebx),
              "=c" (*ecx),
              "=d" (*edx)
            : "0" (*eax), "2" (*ecx));
}

static inline void cpuid(unsigned int op,
                         unsigned int *eax, unsigned int *ebx,
                         unsigned int *ecx, unsigned int *edx)
{
        *eax = op;
        *ecx = 0;
        __cpuid(eax, ebx, ecx, edx);
}

/* Some CPUID calls want 'count' to be placed in ecx */
/*static inline void cpuid_count(unsigned int op, int count,
                               unsigned int *eax, unsigned int *ebx,
                               unsigned int *ecx, unsigned int *edx)
{
        *eax = op;
        *ecx = count;
        __cpuid(eax, ebx, ecx, edx);
}*/

/* Typedef for storing the Cache Information */
typedef union {
   unsigned char ch[48];
   uint32_t      uint[12];
   struct {
      uint32_t    fill1:24;      /* Bit 0 */
      uint32_t    l1_i_sz:8;
      uint32_t    fill2:24; 
      uint32_t    l1_d_sz:8;
      uint32_t    fill3:16; 
      uint32_t    l2_sz:16;
      uint32_t    fill4:18; 
      uint32_t    l3_sz:14;
      uint32_t    fill5[8];
   } amd;
} cpuid_cache_info_t;

/* Typedef for storing the CPUID Vendor String */
typedef union {
   /* Note: the extra byte in the char array is for '\0'. */
   char           char_array[CPUID_VENDOR_STR_LENGTH];
   uint32_t       uint32_array[CPUID_VENDOR_LENGTH];
} cpuid_vendor_string_t;

/* Typedef for storing the CPUID Brand String */
typedef union {
   /* Note: the extra byte in the char array is for '\0'. */
   char           char_array[CPUID_BRAND_STR_LENGTH];
   uint32_t       uint32_array[CPUID_BRAND_LENGTH];
} cpuid_brand_string_t;

/* Typedef for storing CPUID Version */
typedef union {
   uint32_t flat;
   struct {
      uint32_t    stepping:4;      /* Bit 0 */
      uint32_t    model:4;
      uint32_t    family:4;
      uint32_t    processorType:2;
      uint32_t    reserved1514:2;
      uint32_t    extendedModel:4;
      uint32_t    extendedFamily:8;
      uint32_t    reserved3128:4;  /* Bit 31 */
   } bits;      
} cpuid_version_t;

/* Typedef for storing CPUID Processor Information */
typedef union {
   uint32_t flat;
   struct {
      uint32_t    brandIndex:8;    /* Bit 0 */
      uint32_t    cflushLineSize:8;
      uint32_t    logicalProcessorCount:8;
      uint32_t    apicID:8;        /* Bit 31 */
   } bits;      
} cpuid_proc_info_t;

/* Typedef for storing CPUID Feature flags */
typedef union {
   uint32_t flat;
   struct {
      uint32_t    :1;           
   } bits;
} cpuid_custom_features;

/* Typedef for storing CPUID Feature flags */
typedef union {
   uint32_t       uint32_array[3];
   struct {
      uint32_t    fpu:1;           /* EDX feature flags, bit 0 */
      uint32_t    vme:1;
      uint32_t    de:1;
      uint32_t    pse:1;
      uint32_t    rdtsc:1;
      uint32_t    msr:1;
      uint32_t    pae:1;
      uint32_t    mce:1;
      uint32_t    cx8:1;
      uint32_t    apic:1;
      uint32_t    bit10:1;
      uint32_t    sep:1;
      uint32_t    mtrr:1;
      uint32_t    pge:1;
      uint32_t    mca:1;
      uint32_t    cmov:1;
      uint32_t    pat:1;
      uint32_t    pse36:1;
      uint32_t    psn:1;
      uint32_t    cflush:1;
      uint32_t    bit20:1;
      uint32_t    ds:1;
      uint32_t    acpi:1;
      uint32_t    mmx:1;
      uint32_t    fxsr:1;
      uint32_t    sse:1;
      uint32_t    sse2:1;
      uint32_t    ss:1;
      uint32_t    htt:1;
      uint32_t    tm:1;
      uint32_t    bit30:1;
      uint32_t    pbe:1;           /* EDX feature flags, bit 31 */
      uint32_t    sse3:1;          /* ECX feature flags, bit 0 */
      uint32_t    mulq:1;
      uint32_t    bit2:1;
      uint32_t    mon:1;
      uint32_t    dscpl:1;
      uint32_t    vmx:1;
     	uint32_t    smx:1;      	
     	uint32_t    eist:1;  
     	uint32_t    tm2:1;       		     		
      uint32_t    bits_9_31:23;
      uint32_t    bits0_28:29;     /* EDX extended feature flags, bit 0 */
      uint32_t    lm:1;		   /* Long Mode */
      uint32_t    bits_30_31:2;    /* EDX extended feature flags, bit 32 */
   } bits;
} cpuid_feature_flags_t;

/* An overall structure to cache all of the CPUID information */
struct cpu_ident {
	uint32_t max_cpuid;
	uint32_t max_xcpuid;
	uint32_t dts_pmp;
	cpuid_version_t vers;
	cpuid_proc_info_t info;
	cpuid_feature_flags_t fid;
	cpuid_vendor_string_t vend_id;
	cpuid_brand_string_t brand_id;
	cpuid_cache_info_t cache_info;
	cpuid_custom_features custom;
};

struct cpuid4_eax {
	uint32_t	ctype:5;
	uint32_t	level:3;
	uint32_t	is_self_initializing:1;
	uint32_t	is_fully_associative:1;
	uint32_t	reserved:4;
	uint32_t	num_threads_sharing:12;
	uint32_t	num_cores_on_die:6;
};

struct cpuid4_ebx {
	uint32_t	coherency_line_size:12;
	uint32_t	physical_line_partition:10;
	uint32_t	ways_of_associativity:10;
};

struct cpuid4_ecx {
	uint32_t	number_of_sets:32;
};

