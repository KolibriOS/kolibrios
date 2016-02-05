/*
 * All Video Processing kernels 
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

// Modual name: undefall.inc
//
// undefine all global symbol for new process
//

//Source definitions
#undef  ubSRC_Y   
#undef  ubSRC_U   
#undef  ubSRC_V 

#undef  ub2SRC_Y   
#undef  ub2SRC_U   
#undef  ub2SRC_V

#undef  ub4SRC_Y   
#undef  ub4SRC_U   
#undef  ub4SRC_V

#undef  uwSRC_Y   
#undef  uwSRC_U   
#undef  uwSRC_V

#undef  udSRC_Y   
#undef  udSRC_U   
#undef  udSRC_V

#undef  udSRC_YUV
#undef  nSRC_YUV_REG

//Destination definitions
#undef  ubDEST_Y   
#undef  ubDEST_U   
#undef  ubDEST_V 

#undef  ub2DEST_Y   
#undef  ub2DEST_U   
#undef  ub2DEST_V

#undef  ub4DEST_Y   
#undef  ub4DEST_U   
#undef  ub4DEST_V

#undef  uwDEST_Y   
#undef  uwDEST_U   
#undef  uwDEST_V

#undef  udDEST_Y   
#undef  udDEST_U   
#undef  udDEST_V

#undef  udDEST_YUV
#undef  nDEST_YUV_REG
#undef  ubDEST_ARGB

// End of undefall.inc
