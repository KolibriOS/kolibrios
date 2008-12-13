/*
 * Copyright 2007  Luc Verhaegen <lverhaegen@novell.com>
 * Copyright 2007  Matthias Hopf <mhopf@novell.com>
 * Copyright 2007  Egbert Eich   <eich@novell.com>
 * Copyright 2007  Advanced Micro Devices, Inc.
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
#ifndef RHD_I2C_H_
# define RHD_I2C_H_

//#include "xf86DDC.h"
#include "xf86i2c.h"

#define I2C_LINES 5

typedef enum {
    RHD_I2C_INIT,
    RHD_I2C_DDC,
    RHD_I2C_PROBE_ADDR_LINE,
    RHD_I2C_PROBE_ADDR,
    RHD_I2C_GETBUS,
    RHD_I2C_TEARDOWN
} RHDi2cFunc;

typedef union RHDI2CDataArg
{
    I2CBusPtr *I2CBusList;
    int i;
    struct {
	int line;
	CARD8 slave;
    } target;
    struct {
	CARD8 slave;
	I2CBusPtr i2cBusPtr;
    } probe;
    struct
    {
	int line;
    CARD32 slaves[4];
    } scanbus;
    xf86MonPtr monitor;
    I2CBusPtr i2cBusPtr;
} RHDI2CDataArg, *RHDI2CDataArgPtr;

typedef enum {
    RHD_I2C_SUCCESS,
    RHD_I2C_NOLINE,
    RHD_I2C_FAILED
} RHDI2CResult;

RHDI2CResult
RHDI2CFunc(int scrnIndex, I2CBusPtr *I2CList, RHDi2cFunc func,
			RHDI2CDataArgPtr data);
#endif
