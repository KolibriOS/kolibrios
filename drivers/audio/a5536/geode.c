

#define FORCED_PIO

#include "types.h"

#include "pci.h"
#include "syscall.h"

#include "geode.h"

#ifdef DEBUG
  #define DBG(format,...) dbgprintf(format,##__VA_ARGS__)
#else
  #define DBG(format,...)
#endif

#define  BM0_IRQ            0x04
#define  BM1_IRQ            0x08
#define BITS_8_TO_16(x) ( ( (long) ((unsigned char) x - 128) ) << 8 )

#define PCI_VENDOR_ID_NS  0x100b
#define PCI_VENDOR_ID_AMD 0x1022


#ifndef PCI_DEVICE_ID_NS_CS5535_AUDIO
    #define PCI_DEVICE_ID_NS_CS5535_AUDIO 0x002e
#endif
#ifndef PCI_DEVICE_ID_AMD_CS5536_AUDIO
    #define PCI_DEVICE_ID_AMD_CS5536_AUDIO 0x2093
#endif

#define  ID_DEV_1   ((PCI_DEVICE_ID_NS_CS5535_AUDIO  << 16)|PCI_VENDOR_ID_NS)
#define  ID_DEV_2   ((PCI_DEVICE_ID_AMD_CS5536_AUDIO << 16)|PCI_VENDOR_ID_AMD)

#define SET                 1
#define	CLEAR				0

int __stdcall srv_sound(ioctl_t *io);


PRD_ENTRY __attribute__((aligned(16))) prd_tab[5];


typedef struct
{
    PCITAG  pciTag;
    Bool    is_iomapped;
    addr_t  F3BAR0;

    Bool    fAD1819A;

    int     CurrentPowerState;

    addr_t  buffer;
    addr_t  prd_dma;

    addr_t  irq_line;
    u32_t   irq_mask;

    void    __stdcall (*callback)(addr_t buffer);
}geode_t;

geode_t  geode;


static inline void ctrl_write_32(addr_t reg, u32_t data)
{
    reg+= geode.F3BAR0;

#ifdef FORCED_PIO
       out32((u16_t)reg, data);
#else
    if(geode.is_iomapped)
       out32((u16_t)reg, data);
    else
        *(u32_t*)reg = data;
#endif
}

static inline u32_t ctrl_read_32(addr_t reg)
{
    reg+= geode.F3BAR0;
#ifdef FORCED_PIO
        return in32((u16_t)reg);
#else
    if(geode.is_iomapped)
        return in32((u16_t)reg);
    else
        return *(u32_t*)reg;
#endif
}



Bool snd_hw_WaitForBit(addr_t offset, u32_t Bit,
                              unsigned char Operation,
                              count_t timeout,
                              u32_t *pReturnValue)
{
    volatile u32_t  tmp;

    tmp = ctrl_read_32(offset);

    while (timeout)
    {
        if (Operation==CLEAR){
            if (!(tmp & Bit))
                break;
        } else if (tmp & Bit)
                break;

        /*If the Bit is not clear yet, we wait for 10 milisecond and try again*/
        delay(10/10);

        tmp = ctrl_read_32(offset);

        timeout--;
    };

    if (pReturnValue)
        *pReturnValue=tmp;

    if (!timeout)
        return FALSE;

    return TRUE;
}

u16_t snd_hw_CodecRead ( u8_t CodecRegister )
{
    u32_t CodecRegister_data = 0;
    u32_t timeout=10;
    volatile u32_t val=0;

    CodecRegister_data  = ((u32_t)CodecRegister)<<24;
    CodecRegister_data |= 0x80000000;   /* High-bit set (p.106) is a CODEC reg READ.*/

/*  Set the bit.  We are going to access the CODEC...*/
    CodecRegister_data |= BIT_5535_CODEC_COMMAND_NEW;

/*Request the data*/
    ctrl_write_32(CODEC_CONTROL_REG_5535, CodecRegister_data);

/*  Now we need to wait for BIT_5535_CODEC_COMMAND_NEW of the Codec control register to clear
    (For subsequent Reads/Writes)*/
    if (!snd_hw_WaitForBit (CODEC_CONTROL_REG_5535,
                            BIT_5535_CODEC_COMMAND_NEW, CLEAR, 50, NULL))
        DBG("BIT_5535_CODEC_COMMAND_NEW did not clear!!\n");

/* Wait for CODEC_STATUS_NEW and confirm the read of the requested register*/
    timeout = 50;
    do
    {
        val = ctrl_read_32(CODEC_STATUS_REG_5535);
        if ((val & BIT_5535_CODEC_STATUS_NEW) &&
            ((u32_t) CodecRegister == ((0xFF000000 & val)>>24)))
            break;
        else
            /*Wait for 10 miliseconds and try again*/
            delay(10/10);
    } while ( --timeout);

    if (!timeout)
       DBG("Could not read the CODEC!!  Returning what we got.\n");

    return( (u16_t)val );
}

void snd_hw_CodecWrite( u8_t CodecRegister, u16_t CodecData  )
{
    u32_t CodecRegister_data;
    u32_t Temp, timeout;

    CodecRegister_data = ((u32_t) CodecRegister)<<24;
    CodecRegister_data |= (u32_t) CodecData;
    CodecRegister_data &= CODEC_COMMAND_MASK;

    /*Set the bit.  We are going to access the CODEC...*/
    CodecRegister_data |= BIT_5535_CODEC_COMMAND_NEW;

    /*Write the data*/
    ctrl_write_32(CODEC_CONTROL_REG_5535, CodecRegister_data);
    //OS_DbgMsg("Writing: %08X\n", CodecRegister_data);

    /*We need to wait for bit16 of the Codec control register to clear*/
    Temp = ctrl_read_32(CODEC_CONTROL_REG_5535);

    timeout = 50;

    while ((Temp & BIT_5535_CODEC_COMMAND_NEW) && timeout-- )
        Temp = ctrl_read_32(CODEC_CONTROL_REG_5535);

    if (!timeout)
        DBG("Could not Write the CODEC!!\n"
                  "BIT_5535_CODEC_COMMAND_NEW did not clear!\n");
}


void snd_hw_SetCodecRate(u32_t SampleRate)
{
    u16_t val;

    DBG("Rate: %d\n", SampleRate);

    /*If Double-Rate is supported (Bit 2 on register 28h)...*/
    val=snd_hw_CodecRead(EXTENDED_AUDIO_ID);

    if (val & 0x02)
    {
        DBG("Codec supports Double rate.\n");
        val=snd_hw_CodecRead(EXT_AUDIO_CTRL_STAT);

        if (SampleRate>48000)
        {
            snd_hw_CodecWrite(EXT_AUDIO_CTRL_STAT, (u16_t) (val|0x0002));
            SampleRate/=2;
        }
        else
            snd_hw_CodecWrite(EXT_AUDIO_CTRL_STAT, (u16_t) (val&0xFFFD));
    }
    if (geode.fAD1819A)
    {
        DBG("AD1819...\n");
        snd_hw_CodecWrite(AD1819A_PCM_SR0,(u16_t)SampleRate);
    }
    else
        snd_hw_CodecWrite(PCM_FRONT_DAC_RATE,(u16_t)SampleRate);
}

Bool init_device()
{
    u32_t io_base = pciReadLong(geode.pciTag, 0x10);

    if( PCI_MAP_IS_IO(io_base))
    {
        geode.is_iomapped = TRUE;
        geode.F3BAR0 = PCIGETIO(io_base);
        DBG("io mapped F3BAR0 %x\n", geode.F3BAR0);
    }
    else if(PCI_MAP_IS_MEM(io_base))
    {
        geode.is_iomapped = FALSE;
        io_base = PCIGETMEMORY(io_base);
        geode.F3BAR0 = MapIoMem(io_base, 128, PG_SW+PG_NOCACHE);
        DBG("memory mapped F3BAR0 %x\n", geode.F3BAR0);
    }

    geode.buffer = KernelAlloc(64*1024);

    addr_t buffer = geode.buffer;
    addr_t dma = GetPgAddr(geode.buffer);

    geode.prd_dma  = (((addr_t)prd_tab) & 4095) + GetPgAddr((void*)prd_tab);

    prd_tab[0].ulPhysAddr = dma;
    prd_tab[0].SizeFlags = 16384 | PRD_EOP_BIT ;

    prd_tab[1].ulPhysAddr = dma + 16384;
    prd_tab[1].SizeFlags = 16384 | PRD_EOP_BIT ;

    prd_tab[2].ulPhysAddr = dma + 16384*2;
    prd_tab[2].SizeFlags = 16384 | PRD_EOP_BIT ;

    prd_tab[3].ulPhysAddr = dma + 16384*3;
    prd_tab[3].SizeFlags = 16384 | PRD_EOP_BIT ;

    prd_tab[4].ulPhysAddr = geode.prd_dma;
    prd_tab[4].SizeFlags = PRD_JMP_BIT ;

    ctrl_write_32(0x24, geode.prd_dma);

    __clear((void*)buffer,64*1024);
//    u32_t tmp = ctrl_read_32(0x24);

//    dbgprintf("Create primary buffer at %x dma at %x\n", geode.buffer, dma );

//    dbgprintf("Set prd dma %x, read prd dma %x\n", geode.prd_dma, tmp);

    geode.irq_line = pciReadLong(geode.pciTag, 0x3C) & 0xFF;
    geode.irq_mask = ~(1<<geode.irq_line);

    DBG("Irq line %d, mask %x\n", geode.irq_line, geode.irq_mask);


/*
    geode.CommandRegister  = geode.F3BAR0+0x20;
    geode.PRDTableAddress  = geode.F3BAR0+0x24;
    geode.DMAPointer       = geode.F3BAR0+0x60;

    geode.IRQControlRegister        = geode.F3BAR0+0x1C;
    geode.InternalIRQEnableRegister = geode.F3BAR0+0x1A;
    geode.SMI_StatusRegister        = geode.F3BAR0+0x21;
*/



/*CODEC - RESET and volumes initalization.*/
/*Set the Warm RESET and CODEC_COMMAND_NEW bits.*/

    DBG("reset codec...\n");

    ctrl_write_32(CODEC_CONTROL_REG_5535, 0x00030000 );

    if (!snd_hw_WaitForBit (CODEC_STATUS_REG_5535, BIT_CODEC_READY, SET, 40, NULL))
    {
       DBG("Primary Codec NOT Ready...Aborting\n");
       return FALSE;
    }

    /*Check which codec is being used */
    if (snd_hw_CodecRead(AD1819A_VENDORID1) == 0x4144 &&
        snd_hw_CodecRead(AD1819A_VENDORID2) == 0x5303)
    {
        geode.fAD1819A = TRUE;
        /*  Enable non-48kHz sample rates. */
        snd_hw_CodecWrite (AD1819A_SER_CONF,
                           snd_hw_CodecRead(AD1819A_SER_CONF>>8) |
                           AD1819A_SER_CONF_DRQEN);
        DBG("detect AD1819A audio codec\n");
    }
    else
    {
        geode.fAD1819A = FALSE;
        snd_hw_CodecWrite (EXT_AUDIO_CTRL_STAT,
        (snd_hw_CodecRead(EXT_AUDIO_CTRL_STAT) | 0x0001));
        /* set the VRA bit to ON*/
    }

    /* set default volume*/
    snd_hw_CodecWrite( MASTER_VOLUME,       0x0B0B);
    snd_hw_CodecWrite( PCM_OUT_VOL,         0x0808);
    snd_hw_CodecWrite( PC_BEEP_VOLUME,      0x0000);
    snd_hw_CodecWrite( PHONE_VOLUME,        0x8000);
    snd_hw_CodecWrite( MIC_VOLUME,          0x8048);
    snd_hw_CodecWrite( LINE_IN_VOLUME,      0x0808);
    snd_hw_CodecWrite( CD_VOLUME,           0x8000);
    snd_hw_CodecWrite( VIDEO_VOLUME,        0x8000);
    snd_hw_CodecWrite( TV_VOLUME,           0x8000);
    snd_hw_CodecWrite( RECORD_SELECT,       0x0000);
    snd_hw_CodecWrite( RECORD_GAIN,         0x0a0a);
    snd_hw_CodecWrite( GENERAL_PURPOSE,     0x0200);
    snd_hw_CodecWrite( MASTER_VOLUME_MONO,  0x0000);

    snd_hw_SetCodecRate(48000);
    /*Set all the power state bits to 0 (Reg 26h)*/
    snd_hw_CodecWrite (POWERDOWN_CTRL_STAT, 0x0000);
    geode.CurrentPowerState = GEODEAUDIO_D0;
//    OS_DbgMsg("<--snd_hw_InitAudioRegs\n");


    return TRUE;
}

static int snd_StartDMA ()
{

#ifdef FORCED_PIO
        out8( (u16_t)(geode.F3BAR0+0x20),PCI_READS | ENABLE_BUSMASTER);
#else
    if (geode.is_iomapped)
        out8( (u16_t)(geode.F3BAR0+0x20),PCI_READS | ENABLE_BUSMASTER);
    else
        *(u8_t*)(geode.F3BAR0+0x20)= PCI_READS | ENABLE_BUSMASTER;
#endif
    return 0;
};

static u8_t snd_hw_InterruptID ()
{
    volatile u8_t *TempInterruptID, ID;

#ifdef FORCED_PIO
        ID=(u8_t) in16((u16_t)(geode.F3BAR0 + 0x12));
#else
    if (geode.is_iomapped)
        ID=(u8_t) in16((u16_t)(geode.F3BAR0 + 0x12));
    else
    {
        TempInterruptID=(u8_t*)(geode.F3BAR0 + 0x12);
        ID=*TempInterruptID;
    }
#endif
    return (ID);
}


static u8_t snd_hw_ClearStat(int Channel)
{
    volatile u8_t status;      /*Volatile to force read-to-clear.*/

    /*Read to clear*/

#ifdef FORCED_PIO
        status = in8((u16_t) geode.F3BAR0 + 0x21);
#else
    if (geode.is_iomapped)
        status = in8((u16_t) geode.F3BAR0 + 0x21);
    else
        status = *((u8_t*)geode.F3BAR0 + 0x21);
#endif
    return status;
}


void snd_interrupt()
{
    u8_t IntID;

    IntID = snd_hw_InterruptID();

//    dbgprintf("IRQ id %x\n", IntID);

    snd_hw_ClearStat(CHANNEL0_PLAYBACK);
//    snd_hw_ClearStat(CHANNEL1_RECORD);

    if(IntID & BM0_IRQ)
    {
      addr_t prd, offset, base;

      prd = ctrl_read_32(0x24);
      offset = (1 + (prd - geode.prd_dma)>>3) & 3;

      base = geode.buffer + 16384*offset;

      geode.callback(base);
      __asm__ volatile("":::"ebx","esi","edi");

//      dbgprintf(">>BM0_IRQ prd %x offset %x base %x\n", prd, offset, base);
    };
};

Bool FindPciDevice()
{
    u32_t bus, last_bus;
    PCITAG tag;

    if( (last_bus = PciApi(1))==-1)
        return FALSE;

    for(bus=0;bus<=last_bus;bus++)
    {
        u32_t devfn;

        for(devfn=0;devfn<256;devfn++)
        {
            u32_t pciId=0;

            pciId = PciRead32(bus,devfn, 0);

            if( (pciId == ID_DEV_1) ||
                (pciId == ID_DEV_2) )
            {
                DBG("detect companion audio device %x\n", pciId);
                geode.pciTag = pciTag(bus,(devfn>>3)&0x1F,devfn&0x7);
                return TRUE;
            };
        };
    };
    return FALSE;
};


u32_t __stdcall drvEntry(int action)
{
    u32_t retval;

    int i;

    if(action != 1)
        return 0;

#ifdef DEBUG
    if(!dbg_open("/rd/1/drivers/geode.log"))
    {
        printf("Can't open /rd/1/drivers/geode.log\nExit\n");
        return 0;
    }
#endif

    if( FindPciDevice() == FALSE)
    {
        DBG("Device not found\n");
        return 0;
    };

    init_device();

    retval = RegService("SOUND", srv_sound);

    AttachIntHandler(geode.irq_line, snd_interrupt, 0);

    DBG("reg service %s as: %x\n", "SOUND", retval);

    return retval;
};


#define API_VERSION     0x01000100

#define SRV_GETVERSION         0
#define DEV_PLAY               1
#define DEV_STOP               2
#define DEV_CALLBACK           3
#define DEV_SET_BUFF           4
#define DEV_NOTIFY             5
#define DEV_SET_MASTERVOL      6
#define DEV_GET_MASTERVOL      7
#define DEV_GET_INFO           8


int __stdcall srv_sound(ioctl_t *io)
{
    u32_t *inp;
    u32_t *outp;

    inp = io->input;
    outp = io->output;

    switch(io->io_code)
    {
        case SRV_GETVERSION:
            if(io->out_size==4)
            {
                *outp = API_VERSION;
                return 0;
            }
            break;

        case DEV_PLAY:
                return snd_StartDMA();
            break;

        case DEV_STOP:
            break;

        case DEV_CALLBACK:
            if(io->inp_size==4)
            {
                geode.callback = (void*)(*inp);
                return 0;
            }
            break;

    default:
      return ERR_PARAM;
  };
  return ERR_PARAM;
}


