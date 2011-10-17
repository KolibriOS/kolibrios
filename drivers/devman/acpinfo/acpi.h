
#define ACPI_NAME_SIZE                  4
#define ACPI_OEM_ID_SIZE                6
#define ACPI_OEM_TABLE_ID_SIZE          8

typedef struct __attribute__((packed))
{
    u8_t    type;
    u8_t    len;
}acpi_madt_hdr_t;

typedef struct __attribute__((packed))
{
    acpi_madt_hdr_t header;

    u8_t  apic_processor_id;
    u8_t  id;
    struct {
        u32_t enabled   :  1;
        u32_t           : 31;
    } flags;
}acpi_madt_lapic_t;

typedef struct __attribute__((packed))
{
    acpi_madt_hdr_t header;

    u8_t    id;             /* APIC id            */
    u8_t    _rsvd_3;

    u32_t   address;        /* physical address       */
    u32_t   irq_base;       /* global irq number base */
}acpi_madt_ioapic_t;

typedef enum {
    conform_polarity = 0,
    active_high = 1,
    reserved_polarity = 2,
    active_low = 3
}polarity_t ;

typedef enum  {
    conform_trigger = 0,
    edge = 1,
    reserved_trigger = 2,
    level = 3
}trigger_mode_t;

typedef struct __attribute__((packed))
{
    acpi_madt_hdr_t header;

    u8_t    src_bus;        /* source bus, fixed 0=ISA  */
    u8_t    src_irq;        /* source bus irq       */
    u32_t   dest;           /* global irq number        */
    union {
        u16_t   flags;      /* irq flags */
        struct {
            u16_t  polarity      : 2;
            u16_t  trigger_mode  : 2;
            u16_t  reserved      : 12;
        } x;
    };
}acpi_madt_irq_t;

static inline polarity_t irq_get_polarity(acpi_madt_irq_t *irq)
{   return (polarity_t) irq->x.polarity; }

static inline trigger_mode_t irq_get_trigger_mode(acpi_madt_irq_t *irq)
{   return (trigger_mode_t) irq->x.trigger_mode; }


typedef struct __attribute__((packed))
{
    acpi_madt_hdr_t header;

    union {
        u16_t       flags;
        struct {
            u16_t polarity     : 2;
            u16_t trigger_mode : 2;
            u16_t reserved     : 12;
        } x;
    };
    u32_t       irq;

}acpi_madt_nmi_t;

static inline polarity_t nmi_get_polarity(acpi_madt_nmi_t *nmi)
{   return (polarity_t) nmi->x.polarity; }

static inline trigger_mode_t nmi_get_trigger_mode(acpi_madt_nmi_t *nmi)
{   return (trigger_mode_t) nmi->x.trigger_mode; }

typedef struct __attribute__((packed))
{
    acpi_madt_hdr_t header;

    u8_t  apic_processor_id;

    union {
        u16_t       flags;
        struct {
            u16_t polarity     : 2;
            u16_t trigger_mode : 2;
            u16_t reserved     : 12;
        } x;
    };
    u8_t lint;

}acpi_lapic_nmi_t;

static inline polarity_t lapic_nmi_get_polarity(acpi_lapic_nmi_t *nmi)
{   return (polarity_t) nmi->x.polarity; }

static inline trigger_mode_t lapic_nmi_get_trigger_mode(acpi_lapic_nmi_t *nmi)
{   return (trigger_mode_t) nmi->x.trigger_mode; }


typedef struct __attribute__((packed))
{
    u32_t           sig;
    u32_t           len;
    u8_t            rev;
    u8_t            csum;
    char            oem_id[ACPI_OEM_ID_SIZE];
    char            oem_tid[ACPI_OEM_TABLE_ID_SIZE];
    u32_t           oem_rev;
    u32_t           creator_id;
    u32_t           creator_rev;
}acpi_thead_t;


typedef struct __attribute__((packed))
{
    acpi_thead_t    header;
    u32_t           local_apic_addr;
    u32_t           apic_flags;
    u8_t            data[0];
} acpi_madt_t;

typedef struct  __attribute__((packed))
{
    acpi_thead_t    header;
    u32_t           ptrs[0];
}acpi_rsdt_t;

typedef struct  __attribute__((packed))
{
    acpi_thead_t    header;
    u64_t           ptrs[0];
}acpi_xsdt_t;

typedef struct
{
    u32_t           sig[2];
    u8_t            csum;
    char            oemid[6];
    u8_t            rev;
    u32_t           rsdt_ptr;
    u32_t           rsdt_len;
    u64_t           xsdt_ptr;
    u8_t            xcsum;
    u8_t            _rsvd_33[3];
}acpi_rsdp_t;

typedef struct __attribute__((packed))
{
    u8_t            SpaceId;            /* Address space where struct or register exists */
    u8_t            BitWidth;           /* Size in bits of given register */
    u8_t            BitOffset;          /* Bit offset within the register */
    u8_t            AccessWidth;        /* Minimum Access size (ACPI 3.0) */
    u64_t           Address;            /* 64-bit address of struct or register */

} acpi_address_t;

typedef struct __attribute__((packed))
{
    acpi_thead_t    Header;             /* Common ACPI table header */
    u32_t           Facs;               /* 32-bit physical address of FACS */
    u32_t           Dsdt;               /* 32-bit physical address of DSDT */
    u8_t            Model;              /* System Interrupt Model (ACPI 1.0) - not used in ACPI 2.0+ */
    u8_t            PreferredProfile;   /* Conveys preferred power management profile to OSPM. */
    u16_t           SciInterrupt;       /* System vector of SCI interrupt */
    u32_t           SmiCommand;         /* 32-bit Port address of SMI command port */
    u8_t            AcpiEnable;         /* Value to write to smi_cmd to enable ACPI */
    u8_t            AcpiDisable;        /* Value to write to smi_cmd to disable ACPI */
    u8_t            S4BiosRequest;      /* Value to write to SMI CMD to enter S4BIOS state */
    u8_t            PstateControl;      /* Processor performance state control*/
    u32_t           Pm1aEventBlock;     /* 32-bit Port address of Power Mgt 1a Event Reg Blk */
    u32_t           Pm1bEventBlock;     /* 32-bit Port address of Power Mgt 1b Event Reg Blk */
    u32_t           Pm1aControlBlock;   /* 32-bit Port address of Power Mgt 1a Control Reg Blk */
    u32_t           Pm1bControlBlock;   /* 32-bit Port address of Power Mgt 1b Control Reg Blk */
    u32_t           Pm2ControlBlock;    /* 32-bit Port address of Power Mgt 2 Control Reg Blk */
    u32_t           PmTimerBlock;       /* 32-bit Port address of Power Mgt Timer Ctrl Reg Blk */
    u32_t           Gpe0Block;          /* 32-bit Port address of General Purpose Event 0 Reg Blk */
    u32_t           Gpe1Block;          /* 32-bit Port address of General Purpose Event 1 Reg Blk */
    u8_t            Pm1EventLength;     /* Byte Length of ports at Pm1xEventBlock */
    u8_t            Pm1ControlLength;   /* Byte Length of ports at Pm1xControlBlock */
    u8_t            Pm2ControlLength;   /* Byte Length of ports at Pm2ControlBlock */
    u8_t            PmTimerLength;      /* Byte Length of ports at PmTimerBlock */
    u8_t            Gpe0BlockLength;    /* Byte Length of ports at Gpe0Block */
    u8_t            Gpe1BlockLength;    /* Byte Length of ports at Gpe1Block */
    u8_t            Gpe1Base;           /* Offset in GPE number space where GPE1 events start */
    u8_t            CstControl;         /* Support for the _CST object and C States change notification */
    u16_t           C2Latency;          /* Worst case HW latency to enter/exit C2 state */
    u16_t           C3Latency;          /* Worst case HW latency to enter/exit C3 state */
    u16_t           FlushSize;          /* Processor's memory cache line width, in bytes */
    u16_t           FlushStride;        /* Number of flush strides that need to be read */
    u8_t            DutyOffset;         /* Processor duty cycle index in processor's P_CNT reg*/
    u8_t            DutyWidth;          /* Processor duty cycle value bit width in P_CNT register.*/
    u8_t            DayAlarm;           /* Index to day-of-month alarm in RTC CMOS RAM */
    u8_t            MonthAlarm;         /* Index to month-of-year alarm in RTC CMOS RAM */
    u8_t            Century;            /* Index to century in RTC CMOS RAM */
    u16_t           BootFlags;          /* IA-PC Boot Architecture Flags. See Table 5-10 for description */
    u8_t            Reserved;           /* Reserved, must be zero */
    u32_t           Flags;              /* Miscellaneous flag bits (see below for individual flags) */
    acpi_address_t  ResetRegister;      /* 64-bit address of the Reset register */
    u8_t            ResetValue;         /* Value to write to the ResetRegister port to reset the system */
    u8_t            Reserved4[3];       /* Reserved, must be zero */
    u64_t           XFacs;              /* 64-bit physical address of FACS */
    u64_t           XDsdt;              /* 64-bit physical address of DSDT */
    acpi_address_t  XPm1aEventBlock;    /* 64-bit Extended Power Mgt 1a Event Reg Blk address */
    acpi_address_t  XPm1bEventBlock;    /* 64-bit Extended Power Mgt 1b Event Reg Blk address */
    acpi_address_t  XPm1aControlBlock;  /* 64-bit Extended Power Mgt 1a Control Reg Blk address */
    acpi_address_t  XPm1bControlBlock;  /* 64-bit Extended Power Mgt 1b Control Reg Blk address */
    acpi_address_t  XPm2ControlBlock;   /* 64-bit Extended Power Mgt 2 Control Reg Blk address */
    acpi_address_t  XPmTimerBlock;      /* 64-bit Extended Power Mgt Timer Ctrl Reg Blk address */
    acpi_address_t  XGpe0Block;         /* 64-bit Extended General Purpose Event 0 Reg Blk address */
    acpi_address_t  XGpe1Block;         /* 64-bit Extended General Purpose Event 1 Reg Blk address */
} acpi_fadt_t;


#define OS_BASE                          0x80000000

#define ACPI_HI_RSDP_WINDOW_START        (OS_BASE+0x000E0000)
#define ACPI_HI_RSDP_WINDOW_END          (OS_BASE+0x00100000)
#define ACPI_RSDP_CHECKSUM_LENGTH        20
#define ACPI_RSDP_XCHECKSUM_LENGTH       36

#define ACPI_MADT_SIGN                   0x43495041

#define  addr_offset(addr, off) \
    (addr_t)((addr_t)(addr) + (addr_t)(off))

#define  ACPI_ADDR(x) \
    (addr_t)((addr_t)(x)+OS_BASE)

#define acpi_remap(x)      (x)

