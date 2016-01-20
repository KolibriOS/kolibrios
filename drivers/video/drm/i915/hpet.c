#include <syscall.h>
#include <linux/ktime.h>

typedef unsigned int addr_t;

#define ACPI_NAME_SIZE                  4
#define ACPI_OEM_ID_SIZE                6
#define ACPI_OEM_TABLE_ID_SIZE          8

#define ACPI_RSDP_CHECKSUM_LENGTH       20
#define ACPI_RSDP_XCHECKSUM_LENGTH      36

typedef struct __attribute__((packed))
{
    u32           sig;
    u32           len;
    u8            rev;
    u8            csum;
    char          oem_id[ACPI_OEM_ID_SIZE];
    char          oem_tid[ACPI_OEM_TABLE_ID_SIZE];
    u32           oem_rev;
    u32           creator_id;
    u32           creator_rev;
}acpi_thead_t;

typedef struct __attribute__((packed))
{
    u8 space_id;            /* Address space where struct or register exists */
    u8 bit_width;           /* Size in bits of given register */
    u8 bit_offset;          /* Bit offset within the register */
    u8 access_width;        /* Minimum Access size (ACPI 3.0) */
    u64 address;            /* 64-bit address of struct or register */
}acpi_address_t;


typedef struct __attribute__((packed))
{
        acpi_thead_t header;        /* Common ACPI table header */
        u32 id;                     /* Hardware ID of event timer block */
        acpi_address_t address;     /* Address of event timer block */
        u8 sequence;                /* HPET sequence number */
        u16 minimum_tick;           /* Main counter min tick, periodic mode */
        u8 flags;
}acpi_hpet_t;

typedef struct  __attribute__((packed))
{
    acpi_thead_t  header;
    u32           ptrs[0];
}acpi_rsdt_t;

typedef struct  __attribute__((packed))
{
    acpi_thead_t  header;
    u64           ptrs[0];
}acpi_xsdt_t;

typedef struct __attribute__((packed))
{
    u64           sig;
    u8            csum;
    char          oemid[6];
    u8            rev;
    u32           rsdt_ptr;
    u32           rsdt_len;
    u64           xsdt_ptr;
    u8            xcsum;
    u8            _rsvd_33[3];
}acpi_rsdp_t;

#define OS_BASE                 0x80000000
#define ACPI20_PC99_RSDP_START  (OS_BASE + 0x0e0000)
#define ACPI20_PC99_RSDP_END    (OS_BASE + 0x100000)
#define ACPI20_PC99_RSDP_SIZE   (ACPI20_PC99_RSDP_END - ACPI20_PC99_RSDP_START)

static acpi_thead_t* (*sdt_find)(void *sdt, u32 sig);

static u8 acpi_tb_checksum (u8 *buffer, u32 len)
{
    u8    sum = 0;
    u8    *end = buffer + len;

    while (buffer < end)
    {
        sum = (u8)(sum + *(buffer++));
    }

    return sum;
}

static acpi_rsdp_t* acpi_locate()
{
    addr_t p;

    for (p = ACPI20_PC99_RSDP_START; p < ACPI20_PC99_RSDP_END; p+=16)
    {
        acpi_rsdp_t* r = (acpi_rsdp_t*) p;
        if (r->sig != 0x2052545020445352 )
            continue;

        if (acpi_tb_checksum ((u8*)r, ACPI_RSDP_CHECKSUM_LENGTH) != 0)
            continue;

        if ((r->rev >= 2) &&
        (acpi_tb_checksum ((u8*)r, ACPI_RSDP_XCHECKSUM_LENGTH) != 0))
            continue;

        return r;
    };

    return NULL;
};

acpi_thead_t* rsdt_find(acpi_rsdt_t *rsdt, u32 sig)
{
    acpi_thead_t *head = NULL;
    u32 i;

    for (i = 0; i < ((rsdt->header.len-sizeof(acpi_thead_t))/
                sizeof(rsdt->ptrs[0])); i++)
    {
        u32 ptr = rsdt->ptrs[i];

        acpi_thead_t* t = (acpi_thead_t*)MapIoMem(ptr, 8192, PG_SW);

        if (t->sig == sig)
        {
           head = t;
           break;
        };
        FreeKernelSpace(t);
    }
    return head;
};

acpi_thead_t* xsdt_find(acpi_xsdt_t *xsdt, u32 sig)
{
    acpi_thead_t *head = NULL;
    u32 i;

    for (i = 0; i < ((xsdt->header.len-sizeof(acpi_thead_t))/
                sizeof(xsdt->ptrs[0])); i++)
    {
        u32 ptr = xsdt->ptrs[i];

        acpi_thead_t* t = (acpi_thead_t*)MapIoMem(ptr, 8192, PG_SW);

        if (t->sig == sig)
        {
           head = t;
           break;
        };
        FreeKernelSpace(t);
    }
    return head;
};

static void dump_rsdt(acpi_rsdt_t *rsdt)
{
    int i;

    for (i = 0; i < ((rsdt->header.len-sizeof(acpi_thead_t))/
                sizeof(rsdt->ptrs[0])); i++)
    {
        u32 ptr = rsdt->ptrs[i];
        dbgprintf("%s ptr= %p\n", __FUNCTION__, ptr);

        acpi_thead_t* t = (acpi_thead_t*)MapIoMem(ptr, 8192, PG_SW);
        dbgprintf("%s t= %x\n", __FUNCTION__, t);

        char *p = (char*)&t->sig;
        printf("sig %d: %x %c%c%c%c  base %p\n", i, t->sig,
        p[0],p[1],p[2],p[3],rsdt->ptrs[i]);
        FreeKernelSpace(t);
    };
};

typedef struct
{
    u64 hpet_cap;                       /* capabilities */
    u64 res0;                           /* reserved */
    u64 hpet_config;                    /* configuration */
    u64 res1;                           /* reserved */
    u64 hpet_isr;                       /* interrupt status reg */
    u64 res2[25];                       /* reserved */
    union {                             /* main counter */
            volatile u64 _hpet_mc64;
            u32 _hpet_mc32;
            unsigned long _hpet_mc;
    } _u0;
    u64 res3;                           /* reserved */
    struct hpet_timer {
            u64 hpet_config;            /* configuration/cap */
            union {                     /* timer compare register */
                    u64 _hpet_hc64;
                    u32 _hpet_hc32;
                    unsigned long _hpet_compare;
            } _u1;
            u64 hpet_fsb[2];            /* FSB route */
    } hpet_timers[1];
}hpet_t;

#define HPET_ID                 0x000
#define HPET_PERIOD             0x004
#define HPET_CFG                0x010
#define HPET_STATUS             0x020
#define HPET_COUNTER            0x0f0

#define HPET_ID_NUMBER          0x00001f00
#define HPET_ID_NUMBER_SHIFT    8

#define HPET_CFG_ENABLE         0x001

static void *hpet_virt_address;

inline unsigned int hpet_readl(unsigned int a)
{
    return readl(hpet_virt_address + a);
}

static inline void hpet_writel(unsigned int d, unsigned int a)
{
    writel(d, hpet_virt_address + a);
}

static void hpet_start_counter(void)
{
    unsigned int cfg = hpet_readl(HPET_CFG);
    cfg |= HPET_CFG_ENABLE;
    hpet_writel(cfg, HPET_CFG);
}


u64 read_htime()
{
    u32 eflags;
    u64 val;

    eflags = safe_cli();
    asm volatile(
    "1:\n"
    "mov 0xf4(%%ebx), %%edx\n"
    "mov 0xf0(%%ebx), %%eax\n"
    "mov 0xf4(%%ebx), %%ecx\n"
    "cmpl %%edx, %%ecx\n"
    "jnz 1b\n"
    :"=A"(val)
    :"b" (hpet_virt_address)
    :"ecx");
    safe_sti(eflags);
    return val;
}

static u32 period;

void init_hpet()
{
    void        *sdt  = NULL;

    acpi_rsdp_t *rsdp = acpi_locate();

    if (unlikely(rsdp == NULL))
    {
        printf("No ACPI RSD table\n");
        return ;
    };

    printf("rsd base address %p\n", rsdp);

    if(rsdp->rev > 1)
    {
        sdt = (void*)(u32)rsdp->xsdt_ptr;
        sdt_find = xsdt_find;
    }
    else
    {
        sdt = (void*)rsdp->rsdt_ptr;
        sdt_find = rsdt_find;
    };

    printf("sdt address %p\n", sdt);

    if (sdt == NULL)
    {
        printf("Invalid ACPI RSD table\n");
        return ;
    };

    sdt = MapIoMem(sdt, 128*1024, PG_SW);

    printf("sdt mapped address %x\n", sdt);

    acpi_hpet_t *tbl = (acpi_hpet_t*)sdt_find(sdt, 0x54455048);

    u32 hpet_address = tbl->address.address;

    hpet_virt_address = (void*)MapIoMem(hpet_address,1024, PG_SW|0x18);

    printf("hpet address %x mapped at %x\n", hpet_address, hpet_virt_address);

    u32 timers, l, h;

    l = hpet_readl(HPET_ID);
    h = hpet_readl(HPET_PERIOD);
    period = h / 1000000;

    timers = ((l & HPET_ID_NUMBER) >> HPET_ID_NUMBER_SHIFT) + 1;
    printk(KERN_INFO "hpet: ID: 0x%x, PERIOD: 0x%x\n", l, h);
    l = hpet_readl(HPET_CFG);
    h = hpet_readl(HPET_STATUS);
    printk(KERN_INFO "hpet: CFG: 0x%x, STATUS: 0x%x\n", l, h);
    l = hpet_readl(HPET_COUNTER);
    h = hpet_readl(HPET_COUNTER+4);
    printk(KERN_INFO "hpet: COUNTER_l: 0x%x, COUNTER_h: 0x%x\n", l, h);

    hpet_start_counter();

}


