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

#ifndef RHD_BIOSSCRATCH_H_
# define RHD_BIOSSCRATCH_H_

# ifdef ATOM_BIOS

struct BIOSScratchOutputPrivate {
    void (*Mode) (struct rhdOutput *Output, DisplayModePtr Mode);
    void (*Power) (struct rhdOutput *Output, int Power);
    void (*Destroy) (struct rhdOutput *Output);
    struct rhdOutputDevices *OutputDevices;
    enum atomDevice Device;
};

struct rhdAtomOutputDeviceList {
    enum atomDevice DeviceId;
    enum rhdOutputType OutputType;
    enum rhdConnectorType ConnectorType;
};

enum rhdBIOSScratchBlAction {
    rhdBIOSScratchBlGet,
    rhdBIOSScratchBlSet
};


extern struct rhdBiosScratchRegisters *RHDSaveBiosScratchRegisters(RHDPtr rhdPtr);
extern void RHDRestoreBiosScratchRegisters(RHDPtr rhdPtr,
					   struct rhdBiosScratchRegisters * regs);

# if defined (ATOM_BIOS_PARSER)
extern enum rhdSensedOutput RHDBIOSScratchDACSense(struct rhdOutput *Output,
						   struct rhdConnector *Connector);
# endif
extern Bool RHDAtomSetupOutputDriverPrivate(struct rhdAtomOutputDeviceList *Devices,
					    struct rhdOutput *Output);
extern Bool RHDFindConnectorAndOutputTypesForDevice(RHDPtr rhdPtr, enum atomDevice Device,
						    enum rhdOutputType *ot,
						    enum rhdConnectorType *ct);
extern enum atomDevice RHDGetDeviceOnCrtc(RHDPtr rhdPtr, enum atomCrtc Crtc);

extern void RHDAtomBIOSScratchBlLevel(RHDPtr rhdPtr, enum rhdBIOSScratchBlAction action,
				      int *val);

extern void RHDAtomBIOSScratchSetAccelratorMode(RHDPtr rhdPtr, Bool on);
extern void RHDAtomBIOSScratchPMState(RHDPtr rhdPtr, struct rhdOutput *Output,
				      int PowerManagementMode);
# endif /* ATOM_BIOS */
#endif /* RHD_BIOSSCRATCH_H_ */
