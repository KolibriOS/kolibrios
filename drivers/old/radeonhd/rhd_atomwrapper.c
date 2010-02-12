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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "rhd_atomwrapper.h"

#define INT32 INT32
#include "CD_Common_Types.h"
#include "CD_Definitions.h"


int
ParseTableWrapper(void *pspace, int index, void *handle, void *BIOSBase,
		  char **msg_return)
{
    DEVICE_DATA deviceData;
    int ret = 0;

    /* FILL OUT PARAMETER SPACE */
    deviceData.pParameterSpace = (UINT32*) pspace;
    deviceData.CAIL = handle;
    deviceData.pBIOS_Image = BIOSBase;
    deviceData.format = TABLE_FORMAT_BIOS;

    switch (ParseTable(&deviceData, index)) { /* IndexInMasterTable */
	case CD_SUCCESS:
	    ret = 1;
	    *msg_return = "ParseTable said: CD_SUCCESS";
	    break;
	case CD_CALL_TABLE:
	    ret = 1;
	    *msg_return = "ParseTable said: CD_CALL_TABLE";
	    break;
	case CD_COMPLETED:
	    ret = 1;
	    *msg_return = "ParseTable said: CD_COMPLETED";
	    break;
	case CD_GENERAL_ERROR:
	    ret = 0;
	    *msg_return = " ParseTable said: CD_GENERAL_ERROR";
	    break;
	case CD_INVALID_OPCODE:
	    ret = 0;
	    *msg_return = " ParseTable said: CD_INVALID_OPCODE";
	    break;
	case CD_NOT_IMPLEMENTED:
	    ret = 0;
	    *msg_return = " ParseTable said: CD_NOT_IMPLEMENTED";
	    break;
	case CD_EXEC_TABLE_NOT_FOUND:
	    ret = 0;
	    *msg_return = " ParseTable said: CD_EXEC_TABLE_NOT_FOUND";
	    break;
	case CD_EXEC_PARAMETER_ERROR:
	    ret = 0;
	    *msg_return = " ParseTable said: CD_EXEC_PARAMETER_ERROR";
	    break;
	case CD_EXEC_PARSER_ERROR:
	    ret = 0;
	    *msg_return = " ParseTable said: CD_EXEC_PARSER_ERROR";
	    break;
	case CD_INVALID_DESTINATION_TYPE:
	    ret = 0;
	    *msg_return = " ParseTable said: CD_INVALID_DESTINATION_TYPE";
	    break;
	case CD_UNEXPECTED_BEHAVIOR:
	    ret = 0;
	    *msg_return = " ParseTable said: CD_UNEXPECTED_BEHAVIOR";
	    break;
	case CD_INVALID_SWITCH_OPERAND_SIZE:
	    ret = 0;
	    *msg_return = " ParseTable said: CD_INVALID_SWITCH_OPERAND_SIZE\n";
	    break;
    }
    return ret;
}
