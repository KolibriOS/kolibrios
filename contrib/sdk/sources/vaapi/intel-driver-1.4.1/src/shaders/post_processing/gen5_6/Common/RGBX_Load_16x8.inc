/*
 * All Video Processing kernels 
 * Copyright © <2010>, Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Eclipse Public License (EPL), version 1.0.  The full text of the EPL is at
 * http://www.opensource.org/licenses/eclipse-1.0.php.
 *
 */
#ifndef RGBA_LOAD_16X8_INC
#define RGBA_LOAD_16X8_INC

// Module name: RGBA_Load_16x8.inc
//
// RGBA data are first loaded to bottom I/O REGION_2, then does color conversion from RGB to YUV
// finally, YUV data are stored in top I/O REGION_1 with planar format

#undef 	nY_NUM_OF_ROWS

#define nY_NUM_OF_ROWS      8       // Number of Y rows per block

#define nDPR_BLOCK_SIZE_RGBA           nBLOCK_WIDTH_32+nBLOCK_HEIGHT_8   // RGBA block size 32x8 (it is half size)
#define nDPR_MSG_SIZE_RGBA             nRESLEN_8                         // # of MRF's to hold RGBA block data (8)

//Temporary storage for unpacked AYUV data
#define     rUNPACK_TEMP     REG(r,nTEMP0)
.declare    udUNPACK_TEMP    Base=rUNPACK_TEMP    ElementSize=4    SrcRegion=<8;8,1>    Type=ud        //1 GRF
.declare    ubUNPACK_TEMP    Base=rUNPACK_TEMP    ElementSize=1    SrcRegion=<32;32,1>    Type=ub        //1 GRF

.declare ubBOT_Y_IO     Base=REG(r,nBOT_Y) ElementSize=1 SrcRegion=REGION(32,1) Type=ub


#define udSRC_RGBA              udBOT_Y_IO
#define ubSRC_RGBA              ubBOT_Y_IO
#define nSRC_RGBA_REG           nBOT_Y

#define uwDEST_Y                uwTOP_Y
#define uwDEST_U                uwTOP_U
#define uwDEST_V                uwTOP_V

#define SRC_RGBA_OFFSET         a0.0
#define SRC_RGBA_OFFSET_1       a0.0
#define SRC_RGBA_OFFSET_2       a0.1

#define nSRC_REGION nREGION_1    // REGION_1 will be the source region for first kernel

// End of RGBA_Load_16x8.inc
#endif