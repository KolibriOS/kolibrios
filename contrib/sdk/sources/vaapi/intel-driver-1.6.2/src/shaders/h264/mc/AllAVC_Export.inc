/*
 * Copyright © <2010>, Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file was originally licensed under the following license
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */
.export entry_point INTRA_16x16
.export entry_point INTRA_8x8
.export entry_point INTRA_4x4
.export entry_point INTRA_PCM
.export entry_point FRAME_MB
.export entry_point FIELD_MB
.export entry_point MBAFF_MB
#ifdef SW_SCOREBOARD    
.export entry_point SCOREBOARD
.export entry_point SCOREBOARD_MBAFF
#elif defined(HW_SCOREBOARD)
.export entry_point SETHWSCOREBOARD
.export entry_point SETHWSCOREBOARD_MBAFF
#endif	// SW_SCOREBOARD

#ifdef ENABLE_ILDB
.export entry_point AVC_ILDB_ROOT_Y_ILDB_FRAME
.export entry_point AVC_ILDB_CHILD_Y_ILDB_FRAME
.export entry_point AVC_ILDB_ROOT_UV_ILDB_FRAME
.export entry_point AVC_ILDB_CHILD_UV_ILDB_FRAME
.export entry_point AVC_ILDB_ROOT_Y_ILDB_FIELD
.export entry_point AVC_ILDB_CHILD_Y_ILDB_FIELD
.export entry_point AVC_ILDB_ROOT_UV_ILDB_FIELD
.export entry_point AVC_ILDB_CHILD_UV_ILDB_FIELD
.export entry_point AVC_ILDB_ROOT_Y_ILDB_MBAFF
.export entry_point AVC_ILDB_CHILD_Y_ILDB_MBAFF
.export entry_point AVC_ILDB_ROOT_UV_ILDB_MBAFF
.export entry_point AVC_ILDB_CHILD_UV_ILDB_MBAFF
#endif	// ENABLE_ILDB

.export entry_point BSDRESET
.export entry_point DCRESETDUMMY

.export label INTRA_16x16_VERTICAL
.export label INTRA_16x16_HORIZONTAL
.export label INTRA_16x16_DC
.export label INTRA_16x16_PLANE

.export label INTRA_8X8_VERTICAL
.export label INTRA_8X8_HORIZONTAL
.export label INTRA_8X8_DC
.export label INTRA_8X8_DIAG_DOWN_LEFT
.export label INTRA_8X8_DIAG_DOWN_RIGHT
.export label INTRA_8X8_VERT_RIGHT
.export label INTRA_8X8_HOR_DOWN
.export label INTRA_8X8_VERT_LEFT
.export label INTRA_8X8_HOR_UP

.export label INTRA_4X4_VERTICAL
.export label INTRA_4X4_HORIZONTAL
.export label INTRA_4X4_DC
.export label INTRA_4X4_DIAG_DOWN_LEFT
.export label INTRA_4X4_DIAG_DOWN_RIGHT
.export label INTRA_4X4_VERT_RIGHT
.export label INTRA_4X4_HOR_DOWN
.export label INTRA_4X4_VERT_LEFT
.export label INTRA_4X4_HOR_UP

.export label INTRA_CHROMA_DC
.export label INTRA_CHROMA_HORIZONTAL
.export label INTRA_CHROMA_VERTICAL
.export label INTRA_Chroma_PLANE

.export label intra_Pred_4x4_Y
.export label ADD_ERROR_SB0
.export label ADD_ERROR_SB1
.export label ADD_ERROR_SB2
.export label ADD_ERROR_SB3

.export label AllAVC_END

#ifdef SW_SCOREBOARD    
.export label MB_Loop
.export label No_Message
.export label Dependency_Check
.export label Notify_MSG
.export label Update_CurMB
.export label MBAFF_MB_Loop
.export label MBAFF_No_Message
.export label MBAFF_Dependency_Check
.export label MBAFF_Notify_MSG
.export label MBAFF_Update_CurMB

//.export label 

// Definitions for first pass MC kernel building
#ifndef No_Message_IP
#define No_Message_IP	0
#endif

#ifndef Dependency_Check_IP
#define Dependency_Check_IP	0
#endif

#ifndef Notify_MSG_IP
#define Notify_MSG_IP	0
#endif

#ifndef Update_CurMB_IP
#define Update_CurMB_IP	0
#endif

#ifndef MBAFF_No_Message_IP
#define MBAFF_No_Message_IP	0
#endif

#ifndef MBAFF_Dependency_Check_IP
#define MBAFF_Dependency_Check_IP	0
#endif

#ifndef MBAFF_Notify_MSG_IP
#define MBAFF_Notify_MSG_IP	0
#endif

#ifndef	AS_ENABLED
 #ifndef MBAFF_MB_Loop_IP
 #define MBAFF_MB_Loop_IP	0
 #endif

 #ifndef MB_Loop_IP
 #define MB_Loop_IP	0
 #endif
#endif	// End AS_ENABLED

#ifndef MBAFF_Update_CurMB_IP
#define MBAFF_Update_CurMB_IP	0
#endif

#endif	// SW_SCOREBOARD

#ifdef ENABLE_ILDB
.export label	ALL_SPAWNED_UV_ILDB_FRAME
.export label	SLEEP_ENTRY_UV_ILDB_FRAME
.export label	POST_SLEEP_UV_ILDB_FRAME
.export label	ALL_SPAWNED_Y_ILDB_FRAME
.export label	SLEEP_ENTRY_Y_ILDB_FRAME
.export label	POST_SLEEP_Y_ILDB_FRAME

// Definitions for first pass ILDB kernel building
#ifndef	ALL_SPAWNED_UV_ILDB_FRAME_IP
#define	ALL_SPAWNED_UV_ILDB_FRAME_IP	0
#endif

#ifndef	SLEEP_ENTRY_UV_ILDB_FRAME_IP
#define	SLEEP_ENTRY_UV_ILDB_FRAME_IP	0
#endif

#ifndef	POST_SLEEP_UV_ILDB_FRAME_IP
#define	POST_SLEEP_UV_ILDB_FRAME_IP	0
#endif

#ifndef	ALL_SPAWNED_Y_ILDB_FRAME_IP
#define	ALL_SPAWNED_Y_ILDB_FRAME_IP	0
#endif

#ifndef	SLEEP_ENTRY_Y_ILDB_FRAME_IP
#define	SLEEP_ENTRY_Y_ILDB_FRAME_IP	0
#endif

#ifndef	POST_SLEEP_Y_ILDB_FRAME_IP
#define	POST_SLEEP_Y_ILDB_FRAME_IP	0
#endif

#endif	// ENABLE_ILDB
