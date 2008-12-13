/*
 * Copyright 2007, 2008  Egbert Eich   <eich@novell.com>
 * Copyright 2007, 2008  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007, 2008  Matthias Hopf <mhopf@novell.com>
 * Copyright 2007, 2008  Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "xf86.h"
#if HAVE_XF86_ANSIC_H
# include "xf86_ansic.h"
#else
# include <unistd.h>
# include <stdio.h>
#endif

#include "rhd.h"
#include "edid.h"
#include "rhd_i2c.h"
#include "xf86DDC.h"

#include "rhd_regs.h"

#ifdef ATOM_BIOS
#include "rhd_atombios.h"
#endif

#define MAX_I2C_LINES 6

#define RHD_I2C_STATUS_LOOPS 5000

enum rhdDDClines {
    rhdDdc1data = 0,
    rhdDdc2data = 2,
    rhdDdc3data = 4,
    rhdDdc4data = 6, /* arbirarily choosen */
    rhdVIP_DOUT_scl = 0x41,
    rhdDvoData12 = 0x28,
    rhdDdc5data = 0x48,
    rhdDdc6data = 0x4a,
    rhdDdc1clk = 1,
    rhdDdc2clk = 3,
    rhdDdc3clk = 5,
    rhdDdc4clk = 7, /* arbirarily choosen */
    rhdVIP_DOUTvipclk = 0x42,
    rhdDvoData13 = 0x29,
    rhdDdc5clk  = 0x49,
    rhdDdc6clk  = 0x4b,
    rhdDdcUnknown
};

typedef struct _rhdI2CRec
{
    CARD16 prescale;
    union {
	CARD8 line;
	struct i2cGpio {
	    enum rhdDDClines Sda;
	    enum rhdDDClines Scl;
	    CARD32 SdaReg;
	    CARD32 SclReg;
	} Gpio;
    } u;
    int scrnIndex;
} rhdI2CRec;

enum _rhdR6xxI2CBits {
    /* R6_DC_I2C_TRANSACTION0 */
    R6_DC_I2C_RW0   = (0x1 << 0),
    R6_DC_I2C_STOP_ON_NACK0         = (0x1 << 8),
    R6_DC_I2C_ACK_ON_READ0  = (0x1 << 9),
    R6_DC_I2C_START0        = (0x1 << 12),
    R6_DC_I2C_STOP0         = (0x1 << 13),
    R6_DC_I2C_COUNT0        = (0xff << 16),
    /* R6_DC_I2C_TRANSACTION1 */
    R6_DC_I2C_RW1   = (0x1 << 0),
    R6_DC_I2C_STOP_ON_NACK1         = (0x1 << 8),
    R6_DC_I2C_ACK_ON_READ1  = (0x1 << 9),
    R6_DC_I2C_START1        = (0x1 << 12),
    R6_DC_I2C_STOP1         = (0x1 << 13),
    R6_DC_I2C_COUNT1        = (0xff << 16),
    /* R6_DC_I2C_DATA */
    R6_DC_I2C_DATA_RW       = (0x1 << 0),
    R6_DC_I2C_DATA_BIT      = (0xff << 8),
    R6_DC_I2C_INDEX         = (0xff << 16),
    R6_DC_I2C_INDEX_WRITE   = (0x1 << 31),
    /* R6_DC_I2C_CONTROL */
    R6_DC_I2C_GO    = (0x1 << 0),
    R6_DC_I2C_SOFT_RESET    = (0x1 << 1),
    R6_DC_I2C_SEND_RESET    = (0x1 << 2),
    R6_DC_I2C_SW_STATUS_RESET       = (0x1 << 3),
    R6_DC_I2C_SDVO_EN       = (0x1 << 4),
    R6_DC_I2C_SDVO_ADDR_SEL         = (0x1 << 6),
    R6_DC_I2C_DDC_SELECT    = (0x7 << 8),
    R6_DC_I2C_TRANSACTION_COUNT     = (0x3 << 20),
    R6_DC_I2C_SW_DONE_INT   = (0x1 << 0),
    R6_DC_I2C_SW_DONE_ACK   = (0x1 << 1),
    R6_DC_I2C_SW_DONE_MASK  = (0x1 << 2),
    R6_DC_I2C_DDC1_HW_DONE_INT      = (0x1 << 4),
    R6_DC_I2C_DDC1_HW_DONE_ACK      = (0x1 << 5),
    R6_DC_I2C_DDC1_HW_DONE_MASK     = (0x1 << 6),
    R6_DC_I2C_DDC2_HW_DONE_INT      = (0x1 << 8),
    R6_DC_I2C_DDC2_HW_DONE_ACK      = (0x1 << 9),
    R6_DC_I2C_DDC2_HW_DONE_MASK     = (0x1 << 10),
    R6_DC_I2C_DDC3_HW_DONE_INT      = (0x1 << 12),
    R6_DC_I2C_DDC3_HW_DONE_ACK      = (0x1 << 13),
    R6_DC_I2C_DDC3_HW_DONE_MASK     = (0x1 << 14),
    R6_DC_I2C_DDC4_HW_DONE_INT      = (0x1 << 16),
    R6_DC_I2C_DDC4_HW_DONE_ACK      = (0x1 << 17),
    R6_DC_I2C_DDC4_HW_DONE_MASK     = (0x1 << 18),
    /* R6_DC_I2C_SW_STATUS */
    R6_DC_I2C_SW_STATUS_BIT         = (0x3 << 0),
    R6_DC_I2C_SW_DONE       = (0x1 << 2),
    R6_DC_I2C_SW_ABORTED    = (0x1 << 4),
    R6_DC_I2C_SW_TIMEOUT    = (0x1 << 5),
    R6_DC_I2C_SW_INTERRUPTED        = (0x1 << 6),
    R6_DC_I2C_SW_BUFFER_OVERFLOW    = (0x1 << 7),
    R6_DC_I2C_SW_STOPPED_ON_NACK    = (0x1 << 8),
    R6_DC_I2C_SW_SDVO_NACK  = (0x1 << 10),
    R6_DC_I2C_SW_NACK0      = (0x1 << 12),
    R6_DC_I2C_SW_NACK1      = (0x1 << 13),
    R6_DC_I2C_SW_NACK2      = (0x1 << 14),
    R6_DC_I2C_SW_NACK3      = (0x1 << 15),
    R6_DC_I2C_SW_REQ        = (0x1 << 18)
};

enum _rhdR5xxI2CBits {
 /* R5_DC_I2C_STATUS1 */
    R5_DC_I2C_DONE	 = (0x1 << 0),
    R5_DC_I2C_NACK	 = (0x1 << 1),
    R5_DC_I2C_HALT	 = (0x1 << 2),
    R5_DC_I2C_GO	 = (0x1 << 3),
 /* R5_DC_I2C_RESET */
    R5_DC_I2C_SOFT_RESET	 = (0x1 << 0),
    R5_DC_I2C_ABORT	 = (0x1 << 8),
 /* R5_DC_I2C_CONTROL1 */
    R5_DC_I2C_START	 = (0x1 << 0),
    R5_DC_I2C_STOP	 = (0x1 << 1),
    R5_DC_I2C_RECEIVE	 = (0x1 << 2),
    R5_DC_I2C_EN	 = (0x1 << 8),
    R5_DC_I2C_PIN_SELECT	 = (0x3 << 16),
 /* R5_DC_I2C_CONTROL2 */
    R5_DC_I2C_ADDR_COUNT	 = (0x7 << 0),
    R5_DC_I2C_DATA_COUNT	 = (0xf << 8),
    R5_DC_I2C_PRESCALE_LOWER	 = (0xff << 16),
    R5_DC_I2C_PRESCALE_UPPER	 = (0xff << 24),
 /* R5_DC_I2C_CONTROL3 */
    R5_DC_I2C_DATA_DRIVE_EN	 = (0x1 << 0),
    R5_DC_I2C_DATA_DRIVE_SEL	 = (0x1 << 1),
    R5_DC_I2C_CLK_DRIVE_EN	 = (0x1 << 7),
    R5_DC_I2C_RD_INTRA_BYTE_DELAY	 = (0xff << 8),
    R5_DC_I2C_WR_INTRA_BYTE_DELAY	 = (0xff << 16),
    R5_DC_I2C_TIME_LIMIT	 = (0xff << 24),
 /* R5_DC_I2C_DATA */
    R5_DC_I2C_DATA_BIT	 = (0xff << 0),
 /* R5_DC_I2C_INTERRUPT_CONTROL */
    R5_DC_I2C_INTERRUPT_STATUS	 = (0x1 << 0),
    R5_DC_I2C_INTERRUPT_AK	 = (0x1 << 8),
    R5_DC_I2C_INTERRUPT_ENABLE	 = (0x1 << 16),
 /* R5_DC_I2C_ARBITRATION */
    R5_DC_I2C_SW_WANTS_TO_USE_I2C	 = (0x1 << 0),
    R5_DC_I2C_SW_CAN_USE_I2C	 = (0x1 << 1),
    R5_DC_I2C_SW_DONE_USING_I2C	 = (0x1 << 8),
    R5_DC_I2C_HW_NEEDS_I2C	 = (0x1 << 9),
    R5_DC_I2C_ABORT_HDCP_I2C	 = (0x1 << 16),
    R5_DC_I2C_HW_USING_I2C	 = (0x1 << 17)
};

enum _rhdRS69I2CBits {
    /* RS69_DC_I2C_TRANSACTION0 */
    RS69_DC_I2C_RW0   = (0x1 << 0),
    RS69_DC_I2C_STOP_ON_NACK0         = (0x1 << 8),
    RS69_DC_I2C_START0        = (0x1 << 12),
    RS69_DC_I2C_STOP0         = (0x1 << 13),
    /* RS69_DC_I2C_TRANSACTION1 */
    RS69_DC_I2C_RW1   = (0x1 << 0),
    RS69_DC_I2C_START1        = (0x1 << 12),
    RS69_DC_I2C_STOP1         = (0x1 << 13),
    /* RS69_DC_I2C_DATA */
    RS69_DC_I2C_DATA_RW       = (0x1 << 0),
    RS69_DC_I2C_INDEX_WRITE   = (0x1 << 31),
    /* RS69_DC_I2C_CONTROL */
    RS69_DC_I2C_GO    = (0x1 << 0),
    RS69_DC_I2C_TRANSACTION_COUNT     = (0x3 << 20),
    RS69_DC_I2C_SW_DONE_ACK   = (0x1 << 1),
    /* RS69_DC_I2C_SW_STATUS */
    RS69_DC_I2C_SW_DONE       = (0x1 << 2),
    RS69_DC_I2C_SW_ABORTED    = (0x1 << 4),
    RS69_DC_I2C_SW_TIMEOUT    = (0x1 << 5),
    RS69_DC_I2C_SW_INTERRUPTED= (0x1 << 6),
    RS69_DC_I2C_SW_BUFFER_OVERFLOW= (0x1 << 7),
    RS69_DC_I2C_SW_STOPPED_ON_NACK    = (0x1 << 8),
    RS69_DC_I2C_SW_NACK0      = (0x1 << 12),
    RS69_DC_I2C_SW_NACK1      = (0x1 << 13)
};

/* RV620 */
enum rv620I2CBits {
    /* GENERIC_I2C_CONTROL */
    RV62_DC_I2C_GO    = (0x1 << 0),
    RV62_GENERIC_I2C_GO       = (0x1 << 0),
    RV62_GENERIC_I2C_SOFT_RESET       = (0x1 << 1),
    RV62_GENERIC_I2C_SEND_RESET       = (0x1 << 2),
    /* GENERIC_I2C_INTERRUPT_CONTROL */
    RV62_GENERIC_I2C_DONE_INT         = (0x1 << 0),
    RV62_GENERIC_I2C_DONE_ACK         = (0x1 << 1),
    RV62_GENERIC_I2C_DONE_MASK        = (0x1 << 2),
    /* GENERIC_I2C_STATUS */
    RV62_GENERIC_I2C_STATUS_BIT       = (0xf << 0),
    RV62_GENERIC_I2C_DONE     = (0x1 << 4),
    RV62_GENERIC_I2C_ABORTED  = (0x1 << 5),
    RV62_GENERIC_I2C_TIMEOUT  = (0x1 << 6),
    RV62_GENERIC_I2C_STOPPED_ON_NACK  = (0x1 << 9),
    RV62_GENERIC_I2C_NACK     = (0x1 << 10),
    /* GENERIC_I2C_SPEED */
    RV62_GENERIC_I2C_THRESHOLD        = (0x3 << 0),
    RV62_GENERIC_I2C_DISABLE_FILTER_DURING_STALL      = (0x1 << 4),
    RV62_GENERIC_I2C_PRESCALE         = (0xffff << 16),
    /* GENERIC_I2C_SETUP */
    RV62_GENERIC_I2C_DATA_DRIVE_EN    = (0x1 << 0),
    RV62_GENERIC_I2C_DATA_DRIVE_SEL   = (0x1 << 1),
    RV62_GENERIC_I2C_CLK_DRIVE_EN     = (0x1 << 7),
    RV62_GENERIC_I2C_INTRA_BYTE_DELAY         = (0xff << 8),
    RV62_GENERIC_I2C_TIME_LIMIT       = (0xff << 24),
    /* GENERIC_I2C_TRANSACTION */
    RV62_GENERIC_I2C_RW       = (0x1 << 0),
    RV62_GENERIC_I2C_STOP_ON_NACK     = (0x1 << 8),
    RV62_GENERIC_I2C_ACK_ON_READ      = (0x1 << 9),
    RV62_GENERIC_I2C_START    = (0x1 << 12),
    RV62_GENERIC_I2C_STOP     = (0x1 << 13),
    RV62_GENERIC_I2C_COUNT    = (0xf << 16),
    /* GENERIC_I2C_DATA */
    RV62_GENERIC_I2C_DATA_RW  = (0x1 << 0),
    RV62_GENERIC_I2C_DATA_BIT         = (0xff << 8),
    RV62_GENERIC_I2C_INDEX    = (0xf << 16),
    RV62_GENERIC_I2C_INDEX_WRITE      = (0x1 << 31),
    /* GENERIC_I2C_PIN_SELECTION */
    RV62_GENERIC_I2C_SCL_PIN_SEL_SHIFT = 0,
    RV62_GENERIC_I2C_SCL_PIN_SEL      = (0x7f << RV62_GENERIC_I2C_SCL_PIN_SEL_SHIFT),
    RV62_GENERIC_I2C_SDA_PIN_SEL_SHIFT = 8,
    RV62_GENERIC_I2C_SDA_PIN_SEL      = (0x7f << RV62_GENERIC_I2C_SDA_PIN_SEL_SHIFT)
};

/*
 *
 */
static enum rhdDDClines
getDDCLineFromGPIO(int scrnIndex, CARD32 gpio, int shift)
{
    switch (gpio) {
    case 0x1f90:
	switch (shift) {
	    case 0:
		return rhdDdc1clk; /* ddc1 clk */
	    case 8:
		return rhdDdc1data; /* ddc1 data */
	}
	break;
    case 0x1f94: /* ddc2 */
	switch (shift) {
	    case 0:
		return rhdDdc2clk; /* ddc2 clk */
	    case 8:
		return rhdDdc2data; /* ddc2 data */
	}
	break;
    case 0x1f98: /* ddc3 */
	switch (shift) {
	    case 0:
		return rhdDdc3clk; /* ddc3 clk */
	    case 8:
		return rhdDdc3data; /* ddc3 data */
	}
    case 0x1f80: /* ddc4 - on r6xx */
	switch (shift) {
	    case 0:
		return rhdDdc4clk; /* ddc4 clk */
	    case 8:
		return rhdDdc4data; /* ddc4 data */
	}
	break;
    case 0x1f88: /* ddc5 */
	switch (shift) {
	    case 0:
		return rhdVIP_DOUTvipclk; /* ddc5 clk */
	    case 8:
		return rhdVIP_DOUT_scl; /* ddc5 data */
	}
	break;
    case 0x1fda: /* ddc6 */
	switch (shift) {
	    case 0:
		return rhdDvoData13; /* ddc6 clk */
	    case 1:
		return rhdDvoData12; /* ddc6 data */
	}
	break;
    case 0x1fc4:
	switch (shift) {
	    case 0:
		return rhdDdc5clk;
	    case 8:
		return rhdDdc5data;
	}
	break;
    case 0x1fe8: /* ddc6 */
	switch (shift) {
	    case 0:
		return rhdDdc6clk; /* ddc6 clk */
	    case 8:
		return rhdDdc6data; /* ddc6 data */
	}
	break;
    }

    xf86DrvMsg(scrnIndex, X_WARNING,
	       "%s: Failed to match GPIO 0x%04X.%d with a known DDC line\n",
	       __func__, (unsigned int) gpio, shift);
    return rhdDdcUnknown;
}

/*
 *
 */
static Bool
rhdI2CGetDataClkLines(RHDPtr rhdPtr, int line,
		      enum rhdDDClines *scl, enum rhdDDClines *sda,
		      CARD32 *sda_reg, CARD32 *scl_reg)
{
#ifdef ATOM_BIOS
    AtomBiosResult result;
    AtomBiosArgRec data;

    /* scl register */
    data.val = line & 0x0f;
    result = RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			     ATOM_GPIO_I2C_CLK_MASK, &data);
    if (result != ATOM_SUCCESS)
	return FALSE;
    *scl_reg = data.val;

    /* scl DDC line */
	data.val = line & 0x0f;
    result = RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			     ATOM_GPIO_I2C_CLK_MASK_SHIFT, &data);
    if (result != ATOM_SUCCESS)
	return FALSE;
    *scl = getDDCLineFromGPIO(rhdPtr->scrnIndex, *scl_reg, data.val);

    /* sda register */
	    data.val = line & 0x0f;
    result = RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			     ATOM_GPIO_I2C_DATA_MASK, &data);
    if (result != ATOM_SUCCESS)
	return FALSE;
    *sda_reg = data.val;

    /* sda DDC line */
		data.val = line & 0x0f;
    result = RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			     ATOM_GPIO_I2C_DATA_MASK_SHIFT, &data);
    if (result != ATOM_SUCCESS)
	return FALSE;
    *sda = getDDCLineFromGPIO(rhdPtr->scrnIndex, *sda_reg, data.val);

    if ((*scl == rhdDdcUnknown) || (*sda == rhdDdcUnknown)) {
	xf86DrvMsg(rhdPtr->scrnIndex, X_WARNING,
		   "%s: failed to map gpio lines for DDC line %d\n",
		   __func__, line);
	return FALSE;
    }

		    return TRUE;
#else /* ATOM_BIOS */
    return FALSE;
#endif
}

/* R5xx */
static Bool
rhd5xxI2CSetupStatus(I2CBusPtr I2CPtr, int line)
{
    RHDFUNC(I2CPtr);

    line &= 0xf;


    switch (line) {
	case 0:
	    RHDRegMask(I2CPtr, R5_DC_GPIO_DDC1_MASK, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R5_DC_GPIO_DDC1_A, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC1_EN, 0x0, 0xffff);
	    break;
	case 1:
	    RHDRegMask(I2CPtr, R5_DC_GPIO_DDC2_MASK, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R5_DC_GPIO_DDC2_A, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC2_EN, 0x0, 0xffff);
	    break;
	case 2:
	    RHDRegMask(I2CPtr, R5_DC_GPIO_DDC3_MASK, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R5_DC_GPIO_DDC3_A, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R5_DC_GPIO_DDC3_EN, 0x0, 0xffff);
	    break;
	default:
	    xf86DrvMsg(I2CPtr->scrnIndex,X_ERROR,
		       "%s: Trying to initialize non-existent I2C line: %i\n",
		       __func__,line);
	    return FALSE;
    }
    return TRUE;
}

static Bool
rhd5xxI2CStatus(I2CBusPtr I2CPtr)
{
    int count = RHD_I2C_STATUS_LOOPS;
    CARD32 res;

    RHDFUNC(I2CPtr);

    while (count-- != 0) {
	usleep (10);
	if (((RHDRegRead(I2CPtr, R5_DC_I2C_STATUS1)) & R5_DC_I2C_GO) != 0)
	    continue;
	res = RHDRegRead(I2CPtr, R5_DC_I2C_STATUS1);
	RHDDebugVerb(I2CPtr->scrnIndex,1,"SW_STATUS: 0x%x %i\n",
		     (unsigned int)res,count);
	if (res & R5_DC_I2C_DONE)
	    return TRUE;
	else
	    return FALSE;
    }
    RHDRegMask(I2CPtr, R5_DC_I2C_RESET, R5_DC_I2C_ABORT, 0xff00);
    return FALSE;
}

Bool
rhd5xxWriteReadChunk(I2CDevPtr i2cDevPtr, int line, I2CByte *WriteBuffer,
		int nWrite, I2CByte *ReadBuffer, int nRead)
{
    I2CSlaveAddr slave = i2cDevPtr->SlaveAddr;
    I2CBusPtr I2CPtr = i2cDevPtr->pI2CBus;
    rhdI2CPtr I2C = (rhdI2CPtr)(I2CPtr->DriverPrivate.ptr);
    int prescale = I2C->prescale;
    CARD32 save_I2C_CONTROL1, save_494;
    CARD32  tmp32;
    Bool ret = TRUE;

    RHDFUNC(i2cDevPtr->pI2CBus);

    RHDRegMask(I2CPtr, 0x28, 0x200, 0x200);
    save_I2C_CONTROL1 = RHDRegRead(I2CPtr, R5_DC_I2C_CONTROL1);
    save_494 = RHDRegRead(I2CPtr, 0x494);
    RHDRegMask(I2CPtr, 0x494, 1, 1);
    RHDRegMask(I2CPtr, R5_DC_I2C_ARBITRATION,
	       R5_DC_I2C_SW_WANTS_TO_USE_I2C,
	       R5_DC_I2C_SW_WANTS_TO_USE_I2C);

    if (!RHDRegRead(I2CPtr, R5_DC_I2C_ARBITRATION) & R5_DC_I2C_SW_CAN_USE_I2C) {
	RHDDebug(I2CPtr->scrnIndex, "%s SW cannot use I2C line %i\n",__func__,line);
	ret = FALSE;
    } else {

	RHDRegMask(I2CPtr, R5_DC_I2C_STATUS1, R5_DC_I2C_DONE
		   | R5_DC_I2C_NACK
		   | R5_DC_I2C_HALT, 0xff);
	RHDRegMask(I2CPtr, R5_DC_I2C_RESET, R5_DC_I2C_SOFT_RESET, 0xffff);
	RHDRegWrite(I2CPtr, R5_DC_I2C_RESET, 0);

	RHDRegMask(I2CPtr, R5_DC_I2C_CONTROL1,
		   (line  & 0x0f) << 16 | R5_DC_I2C_EN,
		   R5_DC_I2C_PIN_SELECT | R5_DC_I2C_EN);
    }

    if (ret && (nWrite || !nRead)) { /* special case for bus probing */
	/*
	 * chip can't just write the slave address without data.
	 * Add a dummy byte.
	 */
	RHDRegWrite(I2CPtr, R5_DC_I2C_CONTROL2,
		    prescale << 16 |
		    (nWrite ? nWrite : 1) << 8 | 0x01); /* addr_cnt: 1 */
	RHDRegMask(I2CPtr, R5_DC_I2C_CONTROL3,
		   0x30 << 24, 0xff << 24); /* time limit 30 */

	RHDRegWrite(I2CPtr, R5_DC_I2C_DATA, slave);

	/* Add dummy byte */
	if (!nWrite)
	    RHDRegWrite(I2CPtr, R5_DC_I2C_DATA, 0);
	else
	    while (nWrite--)
		RHDRegWrite(I2CPtr, R5_DC_I2C_DATA, *WriteBuffer++);

	RHDRegMask(I2CPtr, R5_DC_I2C_CONTROL1,
		   R5_DC_I2C_START | R5_DC_I2C_STOP, 0xff);
	RHDRegMask(I2CPtr, R5_DC_I2C_STATUS1, R5_DC_I2C_GO, 0xff);

	if ((ret = rhd5xxI2CStatus(I2CPtr)))
	    RHDRegMask(I2CPtr, R5_DC_I2C_STATUS1,R5_DC_I2C_DONE, 0xff);
	else
	    ret = FALSE;
    }

    if (ret && nRead) {

	RHDRegWrite(I2CPtr, R5_DC_I2C_DATA, slave | 1); /*slave*/
	RHDRegWrite(I2CPtr, R5_DC_I2C_CONTROL2,
		    prescale << 16 | nRead << 8 | 0x01); /* addr_cnt: 1 */

	RHDRegMask(I2CPtr, R5_DC_I2C_CONTROL1,
		   R5_DC_I2C_START | R5_DC_I2C_STOP | R5_DC_I2C_RECEIVE, 0xff);
	RHDRegMask(I2CPtr, R5_DC_I2C_STATUS1, R5_DC_I2C_GO, 0xff);
	if ((ret = rhd5xxI2CStatus(I2CPtr))) {
	    RHDRegMask(I2CPtr, R5_DC_I2C_STATUS1, R5_DC_I2C_DONE, 0xff);
	    while (nRead--) {
		*(ReadBuffer++) = (CARD8)RHDRegRead(I2CPtr, R5_DC_I2C_DATA);
	    }
	} else
	    ret = FALSE;
    }

    RHDRegMask(I2CPtr, R5_DC_I2C_STATUS1,
	       R5_DC_I2C_DONE | R5_DC_I2C_NACK | R5_DC_I2C_HALT, 0xff);
    RHDRegMask(I2CPtr, R5_DC_I2C_RESET, R5_DC_I2C_SOFT_RESET, 0xff);
    RHDRegWrite(I2CPtr,R5_DC_I2C_RESET, 0);

    RHDRegMask(I2CPtr,R5_DC_I2C_ARBITRATION,
	       R5_DC_I2C_SW_DONE_USING_I2C, 0xff00);

    RHDRegWrite(I2CPtr,R5_DC_I2C_CONTROL1, save_I2C_CONTROL1);
    RHDRegWrite(I2CPtr,0x494, save_494);
    tmp32 = RHDRegRead(I2CPtr,0x28);
    RHDRegWrite(I2CPtr,0x28, tmp32 & 0xfffffdff);

    return ret;
}

static Bool
rhd5xxWriteRead(I2CDevPtr i2cDevPtr, I2CByte *WriteBuffer, int nWrite, I2CByte *ReadBuffer, int nRead)
{
    /*
     * Since the transaction buffer can only hold
     * 15 bytes (+ the slave address) we bail out
     * on every transaction that is bigger unless
     * it's a read transaction following a write
     * transaction sending just one byte.
     * In this case we assume, that this byte is
     * an offset address. Thus we will restart
     * the transaction after 15 bytes sending
     * a new offset.
     */

    I2CBusPtr I2CPtr = i2cDevPtr->pI2CBus;
    int  ddc_line;

    RHDFUNC(I2CPtr);

    if (nWrite > 15 || (nRead > 15 && nWrite != 1)) {
	xf86DrvMsg(i2cDevPtr->pI2CBus->scrnIndex,X_ERROR,
		   "%s: Currently only I2C transfers with "
		   "maximally 15bytes are supported\n",
		   __func__);
	return FALSE;
    }

    ddc_line = ((rhdI2CPtr)(I2CPtr->DriverPrivate.ptr))->u.line;

    rhd5xxI2CSetupStatus(I2CPtr, ddc_line);

    if (nRead > 15) {
	I2CByte offset = *WriteBuffer;
	while (nRead) {
	    int n = nRead > 15 ? 15 : nRead;
	    if (!rhd5xxWriteReadChunk(i2cDevPtr, ddc_line, &offset, 1, ReadBuffer, n))
		return FALSE;
	    ReadBuffer += n;
	    nRead -= n;
	    offset += n;
	}
	return TRUE;
    } else
	return rhd5xxWriteReadChunk(i2cDevPtr, ddc_line, WriteBuffer, nWrite,
	    ReadBuffer, nRead);
}

/* RS690 */
static Bool
rhdRS69I2CStatus(I2CBusPtr I2CPtr)
{
    volatile CARD32 val;
    int i;

    RHDFUNC(I2CPtr);

    for (i = 0; i < RHD_I2C_STATUS_LOOPS; i++) {
	usleep(10);

	val = RHDRegRead(I2CPtr, RS69_DC_I2C_SW_STATUS);

	RHDDebugVerb(I2CPtr->scrnIndex, 1, "SW_STATUS: 0x%x %i\n",
		     (unsigned int) val, i);

	if (val & RS69_DC_I2C_SW_DONE)
	    break;
    }

    RHDRegMask(I2CPtr, RS69_DC_I2C_INTERRUPT_CONTROL, RS69_DC_I2C_SW_DONE_ACK,
	       RS69_DC_I2C_SW_DONE_ACK);

    if ((i == RHD_I2C_STATUS_LOOPS) ||
	(val & (RS69_DC_I2C_SW_ABORTED | RS69_DC_I2C_SW_TIMEOUT |
		RS69_DC_I2C_SW_INTERRUPTED | RS69_DC_I2C_SW_BUFFER_OVERFLOW |
		RS69_DC_I2C_SW_STOPPED_ON_NACK |
		RS69_DC_I2C_SW_NACK0 | RS69_DC_I2C_SW_NACK1 | 0x3)))
	return FALSE; /* 2 */

    return TRUE; /* 1 */
}

static Bool
rhdRS69I2CSetupStatus(I2CBusPtr I2CPtr, enum rhdDDClines sda, enum rhdDDClines scl, int prescale)
{
    CARD32 clk_pin, data_pin;

    RHDFUNC(I2CPtr);

    switch (sda) {
	case rhdDdc1data:
	    data_pin = 0;
	    break;
	case rhdDdc2data:
	    data_pin = 1;
	    break;
	case rhdDdc3data:
	    data_pin = 2;
	    break;
	default:
	    return FALSE;
    }
    switch (scl) {
	case rhdDdc1data:
	    clk_pin = 4;
	    break;
	case rhdDdc2data:
	    clk_pin = 5;
	    break;
	case rhdDdc3data:
	    clk_pin = 6;
	    break;
	case rhdDdc1clk:
	    clk_pin = 0;
	    break;
	case rhdDdc2clk:
	    clk_pin = 1;
	    break;
	case rhdDdc3clk:
	    clk_pin = 2;
	    break;
	default:
	    return FALSE;
    }

    RHDRegMask(I2CPtr, 0x28, 0x200, 0x200);
    RHDRegMask(I2CPtr, RS69_DC_I2C_UNKNOWN_1, prescale << 16 | 0x2, 0xffff00ff);
    RHDRegWrite(I2CPtr, RS69_DC_I2C_DDC_SETUP_Q, 0x30000000);
    RHDRegMask(I2CPtr, RS69_DC_I2C_CONTROL, ((data_pin & 0x3) << 16) | (clk_pin << 8), 0xffff00);
    RHDRegMask(I2CPtr, RS69_DC_I2C_INTERRUPT_CONTROL, 0x2, 0x2);
    RHDRegMask(I2CPtr, RS69_DC_I2C_UNKNOWN_2, 0x2, 0xff);

    return TRUE;
}

static Bool
rhdRS69WriteRead(I2CDevPtr i2cDevPtr, I2CByte *WriteBuffer,
		 int nWrite, I2CByte *ReadBuffer, int nRead)
{
    Bool ret = FALSE;
    CARD32 data = 0;
    I2CBusPtr I2CPtr = i2cDevPtr->pI2CBus;
    I2CSlaveAddr slave = i2cDevPtr->SlaveAddr;
    rhdI2CPtr I2C = (rhdI2CPtr)I2CPtr->DriverPrivate.ptr;
    int prescale = I2C->prescale;
    int idx = 1;

    enum {
	TRANS_WRITE_READ,
	TRANS_WRITE,
	TRANS_READ
    } trans;

    RHDFUNC(i2cDevPtr->pI2CBus);

    if (nWrite > 0 && nRead > 0) {
	trans = TRANS_WRITE_READ;
    } else if (nWrite > 0) {
	trans = TRANS_WRITE;
    } else if (nRead > 0) {
	trans = TRANS_READ;
    } else {
	/* for bus probing */
	trans = TRANS_WRITE;
    }
    if (slave & 0xff00) {
	xf86DrvMsg(I2CPtr->scrnIndex,X_ERROR,
		   "%s: 10 bit I2C slave addresses not supported\n",__func__);
	return FALSE;
    }

    if (!rhdRS69I2CSetupStatus(I2CPtr, I2C->u.Gpio.Sda, I2C->u.Gpio.Scl,  prescale))
	return FALSE;

    RHDRegMask(I2CPtr, RS69_DC_I2C_CONTROL, (trans == TRANS_WRITE_READ)
	       ? (1 << 20) : 0, RS69_DC_I2C_TRANSACTION_COUNT); /* 2 or 1 Transaction */
    RHDRegMask(I2CPtr, RS69_DC_I2C_TRANSACTION0,
	       RS69_DC_I2C_STOP_ON_NACK0
	       | (trans == TRANS_READ ? RS69_DC_I2C_RW0 : 0)
	       | RS69_DC_I2C_START0
	       | (trans == TRANS_WRITE_READ ? 0 : RS69_DC_I2C_STOP0 )
	       | ((trans == TRANS_READ ? nRead : nWrite)  << 16),
	       0xffffff);
    if (trans == TRANS_WRITE_READ)
	RHDRegMask(I2CPtr, RS69_DC_I2C_TRANSACTION1,
		   nRead << 16
		   | RS69_DC_I2C_RW1
		   | RS69_DC_I2C_START1
		   | RS69_DC_I2C_STOP1,
		   0xffffff); /* <bytes> read */

    data = RS69_DC_I2C_INDEX_WRITE
	| (((slave & 0xfe) | (trans == TRANS_READ ? 1 : 0)) << 8 )
	| (0 << 16);
    RHDRegWrite(I2CPtr, RS69_DC_I2C_DATA, data);
    if (trans != TRANS_READ) { /* we have bytes to write */
	while (nWrite--) {
	    data = RS69_DC_I2C_INDEX_WRITE | ( *(WriteBuffer++) << 8 )
		| (idx++ << 16);
	    RHDRegWrite(I2CPtr, RS69_DC_I2C_DATA, data);
	}
    }
    if (trans == TRANS_WRITE_READ) { /* we have bytes to read after write */
	data = RS69_DC_I2C_INDEX_WRITE | ((slave | 0x1) << 8) | (idx++ << 16);
	RHDRegWrite(I2CPtr, RS69_DC_I2C_DATA, data);
    }
    /* Go! */
    RHDRegMask(I2CPtr, RS69_DC_I2C_CONTROL, RS69_DC_I2C_GO, RS69_DC_I2C_GO);
    if (rhdRS69I2CStatus(I2CPtr)) {
	/* Hopefully this doesn't write data to index */
	RHDRegWrite(I2CPtr, RS69_DC_I2C_DATA, RS69_DC_I2C_INDEX_WRITE
		    | RS69_DC_I2C_DATA_RW  | /* idx++ */3 << 16);
	while (nRead--) {
	    data = RHDRegRead(I2CPtr, RS69_DC_I2C_DATA);
	    *(ReadBuffer++) = (data >> 8) & 0xff;
	}
	ret = TRUE;
    }

    RHDRegMask(I2CPtr, RS69_DC_I2C_CONTROL, 0x2, 0xff);
    usleep(10);
    RHDRegWrite(I2CPtr, RS69_DC_I2C_CONTROL, 0);

    return ret;
}


/* R6xx */
static Bool
rhdR6xxI2CStatus(I2CBusPtr I2CPtr)
{
    volatile CARD32 val;
    int i;

    RHDFUNC(I2CPtr);

    for (i = 0; i < RHD_I2C_STATUS_LOOPS; i++) {
	usleep(10);

	val = RHDRegRead(I2CPtr, R6_DC_I2C_SW_STATUS);

	RHDDebugVerb(I2CPtr->scrnIndex, 1, "SW_STATUS: 0x%x %i\n",
		     (unsigned int) val, i);

	if (val & R6_DC_I2C_SW_DONE)
	    break;
    }

    RHDRegMask(I2CPtr, R6_DC_I2C_INTERRUPT_CONTROL, R6_DC_I2C_SW_DONE_ACK,
	       R6_DC_I2C_SW_DONE_ACK);

    if ((i == RHD_I2C_STATUS_LOOPS) ||
	(val & (R6_DC_I2C_SW_ABORTED | R6_DC_I2C_SW_TIMEOUT |
		R6_DC_I2C_SW_INTERRUPTED | R6_DC_I2C_SW_BUFFER_OVERFLOW |
		R6_DC_I2C_SW_STOPPED_ON_NACK |
		R6_DC_I2C_SW_NACK0 | R6_DC_I2C_SW_NACK1 | 0x3)))
	return FALSE; /* 2 */

    return TRUE; /* 1 */
}

static Bool
rhd6xxI2CSetupStatus(I2CBusPtr I2CPtr, int line, int prescale)
{
    line &= 0xf;

    RHDFUNC(I2CPtr);

    switch (line) {
	case 0:
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC1_MASK, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC1_A, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC1_EN, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_I2C_DDC1_SPEED, (prescale << 16) | 2,
		       0xffff00ff);
	    RHDRegWrite(I2CPtr, R6_DC_I2C_DDC1_SETUP, 0x30000000);
	    break;
	case 1:
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC2_MASK, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC2_A, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC2_EN, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_I2C_DDC2_SPEED, (prescale << 16) | 2,
		       0xffff00ff);
	    RHDRegWrite(I2CPtr, R6_DC_I2C_DDC2_SETUP, 0x30000000);
	    break;
	case 2:
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC3_MASK, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC3_A, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC3_EN, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_I2C_DDC3_SPEED, (prescale << 16) | 2,
		       0xffff00ff);
	    RHDRegWrite(I2CPtr, R6_DC_I2C_DDC3_SETUP, 0x30000000);
	    break;
	case 3:
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC4_MASK, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC4_A, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_GPIO_DDC4_EN, 0x0, 0xffff);
	    RHDRegMask(I2CPtr, R6_DC_I2C_DDC4_SPEED, (prescale << 16) | 2,
		       0xffff00ff);
	    RHDRegWrite(I2CPtr, R6_DC_I2C_DDC4_SETUP, 0x30000000);
	    break;
	default:
	    xf86DrvMsg(I2CPtr->scrnIndex,X_ERROR,
		       "%s: Trying to initialize non-existent I2C line: %i\n",
		       __func__,line);
	    return FALSE;
    }
    RHDRegWrite(I2CPtr, R6_DC_I2C_CONTROL, line << 8);
    RHDRegMask(I2CPtr, R6_DC_I2C_INTERRUPT_CONTROL, 0x2, 0x2);
    RHDRegMask(I2CPtr, R6_DC_I2C_ARBITRATION, 0, 0xff);
    return TRUE;
}

static Bool
rhd6xxWriteRead(I2CDevPtr i2cDevPtr, I2CByte *WriteBuffer, int nWrite, I2CByte *ReadBuffer, int nRead)
{
    Bool ret = FALSE;
    CARD32 data = 0;
    I2CBusPtr I2CPtr = i2cDevPtr->pI2CBus;
    I2CSlaveAddr slave = i2cDevPtr->SlaveAddr;
    rhdI2CPtr I2C = (rhdI2CPtr)I2CPtr->DriverPrivate.ptr;
    CARD32 ddc_line = I2C->u.line;
    int prescale = I2C->prescale;
    int idx = 1;
    enum {
	TRANS_WRITE_READ,
	TRANS_WRITE,
	TRANS_READ
    } trans;

    RHDFUNC(i2cDevPtr->pI2CBus);

    if (nWrite > 0 && nRead > 0) {
	trans = TRANS_WRITE_READ;
    } else if (nWrite > 0) {
	trans = TRANS_WRITE;
    } else if (nRead > 0) {
	trans = TRANS_READ;
    } else {
	/* for bus probing */
	trans = TRANS_WRITE;
    }
    if (slave & 0xff00) {
	xf86DrvMsg(I2CPtr->scrnIndex,X_ERROR,
		   "%s: 10 bit I2C slave addresses not supported\n",__func__);
	return FALSE;
    }

    if (!rhd6xxI2CSetupStatus(I2CPtr, ddc_line,  prescale))
	return FALSE;

    RHDRegMask(I2CPtr, R6_DC_I2C_CONTROL, (trans == TRANS_WRITE_READ)
	       ? (1 << 20) : 0, R6_DC_I2C_TRANSACTION_COUNT); /* 2 or 1 Transaction */
    RHDRegMask(I2CPtr, R6_DC_I2C_TRANSACTION0,
	       R6_DC_I2C_STOP_ON_NACK0
	       | (trans == TRANS_READ ? R6_DC_I2C_RW0 : 0)
	       | R6_DC_I2C_START0
	       | (trans == TRANS_WRITE_READ ? 0 : R6_DC_I2C_STOP0 )
	       | ((trans == TRANS_READ ? nRead : nWrite)  << 16),
	       0xffffff);
    if (trans == TRANS_WRITE_READ)
	RHDRegMask(I2CPtr, R6_DC_I2C_TRANSACTION1,
		   nRead << 16
		   | R6_DC_I2C_RW1
		   | R6_DC_I2C_START1
		   | R6_DC_I2C_STOP1,
		   0xffffff); /* <bytes> read */

    data = R6_DC_I2C_INDEX_WRITE
	| (((slave & 0xfe) | (trans == TRANS_READ ? 1 : 0)) << 8 )
	| (0 << 16);
    RHDRegWrite(I2CPtr, R6_DC_I2C_DATA, data);
    if (trans != TRANS_READ) { /* we have bytes to write */
	while (nWrite--) {
	    data = R6_DC_I2C_INDEX_WRITE | ( *(WriteBuffer++) << 8 )
		| (idx++ << 16);
	    RHDRegWrite(I2CPtr, R6_DC_I2C_DATA, data);
	}
    }
    if (trans == TRANS_WRITE_READ) { /* we have bytes to read after write */
	data = R6_DC_I2C_INDEX_WRITE | ((slave | 0x1) << 8) | (idx++ << 16);
	RHDRegWrite(I2CPtr, R6_DC_I2C_DATA, data);
    }
    /* Go! */
    RHDRegMask(I2CPtr, R6_DC_I2C_CONTROL, R6_DC_I2C_GO, R6_DC_I2C_GO);
    if (rhdR6xxI2CStatus(I2CPtr)) {
	/* Hopefully this doesn't write data to index */
	RHDRegWrite(I2CPtr, R6_DC_I2C_DATA, R6_DC_I2C_INDEX_WRITE
		    | R6_DC_I2C_DATA_RW  | /* idx++ */3 << 16);
	while (nRead--) {
	    data = RHDRegRead(I2CPtr, R6_DC_I2C_DATA);
	    *(ReadBuffer++) = (data >> 8) & 0xff;
	}
	ret = TRUE;
    }

    RHDRegMask(I2CPtr, R6_DC_I2C_CONTROL, 0x2, 0xff);
    usleep(10);
    RHDRegWrite(I2CPtr, R6_DC_I2C_CONTROL, 0);

    return ret;
}

/* RV620 */
static Bool
rhdRV620I2CStatus(I2CBusPtr I2CPtr)
{
    volatile CARD32 val;
    int i;

    RHDFUNC(I2CPtr);

    for (i = 0; i < RHD_I2C_STATUS_LOOPS; i++) {
	usleep(10);

	val = RHDRegRead(I2CPtr, RV62_GENERIC_I2C_STATUS);

	RHDDebugVerb(I2CPtr->scrnIndex, 1,
		     "SW_STATUS: 0x%x %i\n", (unsigned int) val, i);
	if (val & RV62_GENERIC_I2C_DONE)
	    break;
    }

    RHDRegMask(I2CPtr, RV62_GENERIC_I2C_INTERRUPT_CONTROL, 0x2, 0xff);

    if ((i == RHD_I2C_STATUS_LOOPS) ||
	(val & (RV62_GENERIC_I2C_STOPPED_ON_NACK | RV62_GENERIC_I2C_NACK |
		RV62_GENERIC_I2C_ABORTED | RV62_GENERIC_I2C_TIMEOUT)))
	return FALSE; /* 2 */

    return TRUE; /* 1 */
}

/*
 *
 */
static  Bool
rhdRV620I2CSetupStatus(I2CBusPtr I2CPtr, struct i2cGpio *Gpio, int prescale)
{
    CARD32 reg_7d9c = 0; /* 0 is invalid */
    CARD32 scl_reg;

    RHDFUNC(I2CPtr);

    scl_reg = Gpio->SclReg;
    reg_7d9c = (Gpio->Scl << RV62_GENERIC_I2C_SCL_PIN_SEL_SHIFT)
	| (Gpio->Sda << RV62_GENERIC_I2C_SDA_PIN_SEL_SHIFT);

    scl_reg = Gpio->SclReg;
    /* Don't understand this yet */
    if (scl_reg == 0x1fda)
	scl_reg = 0x1f90;

    RHDRegWrite(I2CPtr, scl_reg << 2, 0);

    RHDRegWrite(I2CPtr, RV62_GENERIC_I2C_PIN_SELECTION, reg_7d9c);
    RHDRegMask(I2CPtr, RV62_GENERIC_I2C_SPEED,
	    (prescale & 0xffff) << 16 | 0x02, 0xffff00ff);
    RHDRegWrite(I2CPtr, RV62_GENERIC_I2C_SETUP, 0x30000000);
    RHDRegMask(I2CPtr, RV62_GENERIC_I2C_INTERRUPT_CONTROL,
	    RV62_GENERIC_I2C_DONE_ACK, RV62_GENERIC_I2C_DONE_ACK);

    return TRUE;
}

static Bool
rhdRV620Transaction(I2CDevPtr i2cDevPtr, Bool Write, I2CByte *Buffer, int count)
{
    I2CBusPtr I2CPtr = i2cDevPtr->pI2CBus;
    I2CSlaveAddr slave = i2cDevPtr->SlaveAddr;
    Bool Start = TRUE;

    RHDFUNC(I2CPtr);

#define MAX 8

    while (count > 0 || (Write && Start)) {
	int num;
	int idx = 0;
	CARD32 data = 0;

	if (count > MAX) {
	    num = MAX;
	    RHDRegMask(I2CPtr, RV62_GENERIC_I2C_TRANSACTION,
		    (MAX - (((Start) ? 0 : 1))) << 16
		    | RV62_GENERIC_I2C_STOP_ON_NACK
		    | RV62_GENERIC_I2C_ACK_ON_READ
		    | (Start ? RV62_GENERIC_I2C_START : 0)
		    | (!Write ? RV62_GENERIC_I2C_RW : 0 ),
		    0xFFFFFF);
	} else {
	    num = count;
	    data = ( count - (((Start) ? 0 : 1)) ) << 16
		| RV62_GENERIC_I2C_STOP_ON_NACK
		|  RV62_GENERIC_I2C_STOP
		| (Start ? RV62_GENERIC_I2C_START : 0)
		| (!Write ? RV62_GENERIC_I2C_RW : 0);
	    RHDRegMask(I2CPtr, RV62_GENERIC_I2C_TRANSACTION,
		    data,
		    0xFFFFFF);
	}

	if (Start) {
	    data = RV62_GENERIC_I2C_INDEX_WRITE
		| (((slave & 0xfe) | ( Write ? 0 : 1)) << 8)
		| (idx++ << 16);
	    RHDRegWrite(I2CPtr, RV62_GENERIC_I2C_DATA, data);
	}

	if (Write) {
	    while (num--) {
		data = RV62_GENERIC_I2C_INDEX_WRITE
		    | (idx++ << 16)
		    | *(Buffer++) << 8;
		RHDRegWrite(I2CPtr, RV62_GENERIC_I2C_DATA, data);
	    }

	    RHDRegMask(I2CPtr, RV62_GENERIC_I2C_CONTROL,
		    RV62_GENERIC_I2C_GO, RV62_GENERIC_I2C_GO);
	    if (!rhdRV620I2CStatus(I2CPtr))
		return FALSE;
	} else {

	    RHDRegMask(I2CPtr, RV62_GENERIC_I2C_CONTROL,
		    RV62_GENERIC_I2C_GO, RV62_GENERIC_I2C_GO);
	    if (!rhdRV620I2CStatus(I2CPtr))
		return FALSE;

	    RHDRegWrite(I2CPtr, RV62_GENERIC_I2C_DATA,
		     RV62_GENERIC_I2C_INDEX_WRITE
		     | (idx++ << 16)
		     | RV62_GENERIC_I2C_RW);

	    while (num--) {
		data = RHDRegRead(I2CPtr, RV62_GENERIC_I2C_DATA);
		*(Buffer++) = (CARD8)((data >> 8) & 0xff);
	    }
	}
	Start = FALSE;
	count -= MAX;
    }

    return TRUE;
}

static Bool
rhdRV620WriteRead(I2CDevPtr i2cDevPtr, I2CByte *WriteBuffer, int nWrite, I2CByte *ReadBuffer, int nRead)
{
    I2CBusPtr I2CPtr = i2cDevPtr->pI2CBus;
    rhdI2CPtr I2C = (rhdI2CPtr)I2CPtr->DriverPrivate.ptr;
    int prescale = I2C->prescale;

    RHDFUNC(I2C);

    rhdRV620I2CSetupStatus(I2CPtr, &I2C->u.Gpio, prescale);

    if (nWrite || !nRead)
	if (!rhdRV620Transaction(i2cDevPtr, TRUE, WriteBuffer, nWrite))
	    return FALSE;
    if (nRead)
	if (!rhdRV620Transaction(i2cDevPtr, FALSE, ReadBuffer, nRead))
	    return FALSE;

    return TRUE;
}

static void
rhdTearDownI2C(I2CBusPtr *I2C)
{
    int i;

    /*
     * xf86I2CGetScreenBuses() is
     * broken in older server versions.
     * So we cannot use it. How bad!
     */
    for (i = 0; i < I2C_LINES; i++) {
	char *name;
	if (!I2C[i])
	    break;
	name = I2C[i]->BusName;
	xfree(I2C[i]->DriverPrivate.ptr);
	xf86DestroyI2CBusRec(I2C[i], TRUE, TRUE);
	xfree(name);
    }
    xfree(I2C);
}

#define TARGET_HW_I2C_CLOCK 25 /*  kHz */
#define DEFAULT_ENGINE_CLOCK 453000 /* kHz (guessed) */
#define DEFAULT_REF_CLOCK 27000

static CARD32
rhdGetI2CPrescale(RHDPtr rhdPtr)
{
#ifdef ATOM_BIOS
    AtomBiosArgRec atomBiosArg;
    RHDFUNC(rhdPtr);

    if (rhdPtr->ChipSet < RHD_R600) {
    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    GET_DEFAULT_ENGINE_CLOCK, &atomBiosArg)
	    == ATOM_SUCCESS)
	    return (0x7f << 8)
		+ (atomBiosArg.val / (4 * 0x7f * TARGET_HW_I2C_CLOCK));
	else
	    return (0x7f << 8)
		+ (DEFAULT_ENGINE_CLOCK / (4 * 0x7f * TARGET_HW_I2C_CLOCK));
    } else if (rhdPtr->ChipSet < RHD_RV620) {
    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    GET_REF_CLOCK, &atomBiosArg) == ATOM_SUCCESS)
	    return (atomBiosArg.val / TARGET_HW_I2C_CLOCK);
	else
	    return (DEFAULT_REF_CLOCK / TARGET_HW_I2C_CLOCK);
    } else {
    if (RHDAtomBiosFunc(rhdPtr, rhdPtr->atomBIOS,
			    GET_REF_CLOCK, &atomBiosArg) == ATOM_SUCCESS)
	    return (atomBiosArg.val / (4 * TARGET_HW_I2C_CLOCK));
	else
	    return (DEFAULT_REF_CLOCK / (4 * TARGET_HW_I2C_CLOCK));
    }
#else
    RHDFUNC(rhdPtr);

    if (rhdPtr->ChipSet < RHD_R600) {
	return (0x7f << 8)
	    + (DEFAULT_ENGINE_CLOCK) / (4 * 0x7f * TARGET_HW_I2C_CLOCK);
    } else if (rhdPtr->ChipSet < RHD_RV620) {
	return (DEFAULT_REF_CLOCK / TARGET_HW_I2C_CLOCK);
    } else
	  return (DEFAULT_REF_CLOCK / (4 * TARGET_HW_I2C_CLOCK));
#endif
}

static Bool
rhdI2CAddress(I2CDevPtr d, I2CSlaveAddr addr)
{
    d->SlaveAddr = addr;
    return xf86I2CWriteRead(d, NULL, 0, NULL, 0);
}

/*
 * This stub is needed to keep xf86I2CProbeAddress() happy.
 */
static void
rhdI2CStop(I2CDevPtr d)
{
}

static I2CBusPtr *
rhdInitI2C(int scrnIndex)
{
    int i;
    rhdI2CPtr I2C;
    I2CBusPtr I2CPtr = NULL;
    RHDPtr rhdPtr = (RHDPtr)scrnIndex;
    I2CBusPtr *I2CList;
    int numLines;
    CARD16 prescale = rhdGetI2CPrescale(rhdPtr);
    enum rhdDDClines sda = 0, scl = 0;
    CARD32 scl_reg = 0, sda_reg = 0;
    Bool valid;

    RHDFUNCI(scrnIndex);

    if (rhdPtr->ChipSet < RHD_RS600)
	numLines = 3;
    else if (rhdPtr->ChipSet < RHD_R600)
	numLines = 4;
    else if (rhdPtr->ChipSet < RHD_RV730)
	numLines = 4;
    else
	numLines = MAX_I2C_LINES;

    if (!(I2CList = xcalloc(MAX_I2C_LINES, sizeof(I2CBusPtr)))) {
	xf86DrvMsg(scrnIndex, X_ERROR,
		   "%s: Out of memory.\n",__func__);
    }
    /* We have 4 I2C lines */
    for (i = 0; i < numLines; i++) {
	if (!(I2C = xcalloc(sizeof(rhdI2CRec),1))) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "%s: Out of memory.\n",__func__);
	    goto error;
	}
	I2C->scrnIndex = scrnIndex;

	valid = rhdI2CGetDataClkLines(rhdPtr, i, &scl, &sda, &sda_reg, &scl_reg);
	if (rhdPtr->ChipSet < RHD_RS600
	    || (rhdPtr->ChipSet > RHD_RS740 && rhdPtr->ChipSet < RHD_RV620)) {

	    if (valid) {
		if (sda == rhdDdc1data && scl == rhdDdc1clk)
		    I2C->u.line = 0;
		else if (sda == rhdDdc2data && scl == rhdDdc2clk)
		    I2C->u.line = 1;
		else if (sda == rhdDdc3data && scl == rhdDdc3clk)
		    I2C->u.line = 2;
		else if (rhdPtr->ChipSet > RHD_RS740 && sda == rhdDdc4data && scl == rhdDdc4clk)
		    I2C->u.line = 3; /* R6XX only */
		else {
		    xf86DrvMsg(I2CPtr->scrnIndex, X_ERROR, "No DDC line found for index %i: scl=0x%2.2x sda=0x%2.2x\n",
			       i, scl, sda);
		    xfree(I2C);
		    continue;
		}

	    } else
		I2C->u.line = i;

	} else if (rhdPtr->ChipSet <= RHD_RS740) {

	    if (valid) {
		if (sda != rhdDdc1data && sda != rhdDdc2data && sda != rhdDdc3data) {
		    xf86DrvMsg(I2CPtr->scrnIndex, X_ERROR, "Invalid DDC CLK pin found: %i\n",
			       sda);
		    xfree(I2C);
		    continue;
		}
		if (scl != rhdDdc1data && scl != rhdDdc2data && scl != rhdDdc3data
		    && scl != rhdDdc1clk && scl != rhdDdc2clk && scl != rhdDdc3clk) {
		    xf86DrvMsg(I2CPtr->scrnIndex, X_ERROR, "Invalid DDC CLK pin found: %i\n",
			       scl);
		    xfree(I2C);
		    continue;
		}
		I2C->u.Gpio.Sda = sda;
		I2C->u.Gpio.Scl = scl;
		I2C->u.Gpio.SdaReg = sda_reg;
		I2C->u.Gpio.SclReg = scl_reg;

	    } else {
		xf86DrvMsg(I2CPtr->scrnIndex, X_ERROR, "Invalid ClkLine for DDC. "
			   "AtomBIOS reported wrong or AtomBIOS unavailable\n");
		xfree(I2C);
		goto error;
	    }

	} else {

	    if (valid) {
		I2C->u.Gpio.Sda = sda;
		I2C->u.Gpio.Scl = scl;
		I2C->u.Gpio.SdaReg = sda_reg;
		I2C->u.Gpio.SclReg = scl_reg;
	    } else {
		CARD32 gpioReg[] = { 0x1f90, 0x1f94, 0x1f98 };
		enum rhdDDClines sdaList[] = { rhdDdc1data, rhdDdc2data, rhdDdc3data };
		enum rhdDDClines sclList[] = { rhdDdc1clk, rhdDdc2clk, rhdDdc3clk };
		if (i > 2) {
		    xfree(I2C);
		    continue;
		}
		I2C->u.Gpio.Sda = sdaList[i];
		I2C->u.Gpio.Scl = sclList[i];
		I2C->u.Gpio.SclReg = I2C->u.Gpio.SdaReg = gpioReg[i];
	    }

	}

        /*
	 * This is a value that has been found to work on many cards.
	 * It nees to be replaced by the proper calculation formula
	 * once this is available.
	 */
	I2C->prescale = prescale;
	xf86DrvMsgVerb(scrnIndex, X_INFO, 5, "I2C clock prescale value: %x\n",I2C->prescale);

	if (!(I2CPtr = xf86CreateI2CBusRec())) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Cannot allocate I2C BusRec.\n");
	    xfree(I2C);
	    goto error;
	}
	I2CPtr->DriverPrivate.ptr = I2C;
	if (!(I2CPtr->BusName = xalloc(18))) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "%s: Cannot allocate memory.\n",__func__);
	    xfree(I2C);
	    xf86DestroyI2CBusRec(I2CPtr, TRUE, FALSE);
	    goto error;
	}
	snprintf(I2CPtr->BusName,17,"RHD I2C line %1.1i",i);
	I2CPtr->scrnIndex = scrnIndex;
	if (rhdPtr->ChipSet < RHD_RS600)
	    I2CPtr->I2CWriteRead = rhd5xxWriteRead;
	else if (rhdPtr->ChipSet >= RHD_RS600 && rhdPtr->ChipSet <= RHD_RS740)
	    I2CPtr->I2CWriteRead = rhdRS69WriteRead;
	else if (rhdPtr->ChipSet < RHD_RV620)
	    I2CPtr->I2CWriteRead = rhd6xxWriteRead;
	else
	    I2CPtr->I2CWriteRead = rhdRV620WriteRead;
	I2CPtr->I2CAddress = rhdI2CAddress;
	I2CPtr->I2CStop = rhdI2CStop;

	if (!(xf86I2CBusInit(I2CPtr))) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "I2C BusInit failed for bus %i\n",i);
	    xfree(I2CPtr->BusName);
	    xfree(I2C);
	    xf86DestroyI2CBusRec(I2CPtr, TRUE, FALSE);
	    goto error;
	}
	I2CList[i] = I2CPtr;
    }
    return I2CList;
 error:
    rhdTearDownI2C(I2CList);
    return NULL;
}

RHDI2CResult
rhdI2CProbeAddress(int scrnIndex, I2CBusPtr I2CBusPtr, CARD8 slave)
{
    I2CDevPtr dev;
    char *name = "I2CProbe";

    if ((dev = xf86CreateI2CDevRec())) {
	dev->DevName = name;
	dev->pI2CBus = I2CBusPtr;

	if (xf86I2CDevInit(dev)) {
	    Bool ret;

	    dev->SlaveAddr = slave & 0xFE;

	    ret = xf86I2CWriteRead(dev, NULL, 0, NULL, 0);

	    if (ret) {
		unsigned char offset = 0;
		unsigned char buf[2];

		/*
		  ASUS M2A-VM (R690) motherboards ACK all I2C slaves on the
		  HDMI line when the HDMI riser card is not installed.
		  We therefore need to read the first two bytes and check
		  if they are part of an I2C header.
		*/
		ret = xf86I2CWriteRead(dev, &offset, 1, buf, 2);
		if (ret && (buf[0] != 0 || buf[1] != 0xff))
		    ret = FALSE;
	    }
	    xf86DestroyI2CDevRec(dev, TRUE);

	    return ret ? RHD_I2C_SUCCESS : RHD_I2C_FAILED;
	}
    }

    return RHD_I2C_FAILED;
}

RHDI2CResult
RHDI2CFunc(int scrnIndex, I2CBusPtr *I2CList, RHDi2cFunc func,
	   RHDI2CDataArgPtr datap)
{
    RHDFUNCI(scrnIndex);

    if (func == RHD_I2C_INIT) {
	if (!(datap->I2CBusList = rhdInitI2C(scrnIndex)))
	    return RHD_I2C_FAILED;
	else
	    return RHD_I2C_SUCCESS;
    }
    if (func == RHD_I2C_DDC) {
	if (datap->i >= MAX_I2C_LINES || !I2CList[datap->i])
	    return RHD_I2C_NOLINE;

	datap->monitor = xf86DoEDID_DDC2(scrnIndex, I2CList[datap->i]);
	return RHD_I2C_SUCCESS;
    }
    if (func == RHD_I2C_PROBE_ADDR_LINE) {

	if (datap->target.line >= MAX_I2C_LINES || !I2CList[datap->target.line])
	    return RHD_I2C_NOLINE;
	return rhdI2CProbeAddress(scrnIndex, I2CList[datap->target.line], datap->target.slave);
    }
    if (func == RHD_I2C_PROBE_ADDR) {
	return rhdI2CProbeAddress(scrnIndex, datap->probe.i2cBusPtr, datap->probe.slave);
    }
    if (func == RHD_I2C_GETBUS) {
	if (datap->i >= MAX_I2C_LINES || !I2CList[datap->i])
	    return RHD_I2C_NOLINE;

	datap->i2cBusPtr = I2CList[datap->i];
	return RHD_I2C_SUCCESS;
    }
    if (func == RHD_I2C_TEARDOWN) {
	if (I2CList)
	    rhdTearDownI2C(I2CList);
	return RHD_I2C_SUCCESS;
    }
    return RHD_I2C_FAILED;
}

