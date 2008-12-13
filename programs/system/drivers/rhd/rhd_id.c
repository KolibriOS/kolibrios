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
#include "config.h"
#endif

#include "xf86.h"
#include "rhd.h"
#ifdef ATOM_BIOS
#include "rhd_atombios.h"
#endif
#include "rhd_connector.h"
#include "rhd_output.h"
#include "rhd_card.h"

SymTabRec RHDChipsets[] = {
    /* R500 */
    { RHD_RV505, "RV505" },
    { RHD_RV515, "RV515" },
    { RHD_RV516, "RV516" },
    { RHD_R520,  "R520" },
    { RHD_RV530, "RV530" },
    { RHD_RV535, "RV535" },
    { RHD_RV550, "RV550" },
    { RHD_RV560, "RV560" },
    { RHD_RV570, "RV570" },
    { RHD_R580,  "R580" },
    /* R500 Mobility */
    { RHD_M52,   "M52" },
    { RHD_M54,   "M54" },
    { RHD_M56,   "M56" },
    { RHD_M58,   "M58" },
    { RHD_M62,   "M62" },
    { RHD_M64,   "M64" },
    { RHD_M66,   "M66" },
    { RHD_M68,   "M68" },
    { RHD_M71,   "M71" },
    /* R500 integrated */
    { RHD_RS600, "RS600" },
    { RHD_RS690, "RS690" },
    { RHD_RS740, "RS740" },
    /* R600 */
    { RHD_R600,  "R600" },
    { RHD_RV610, "RV610" },
    { RHD_RV630, "RV630" },
    /* R600 Mobility */
    { RHD_M72,   "M72" },
    { RHD_M74,   "M74" },
    { RHD_M76,   "M76" },
    /* RV670 came into existence after RV6x0 and M7x */
    { RHD_RV670, "RV670" },
    { RHD_M88,   "M88" },
    { RHD_R680,  "R680"  },
    { RHD_RV620, "RV620" },
    { RHD_M82,   "M82"   },
    { RHD_RV635, "RV635" },
    { RHD_M86,   "M86"   },
    { RHD_RS780, "RS780" },
    { RHD_RV770, "RV770" },
    { RHD_RV730, "RV730" },
    { RHD_RV710, "RV710" },
    { -1,      NULL }
};

# define RHD_DEVICE_MATCH(d, i) { (d),(i) }
# define PCI_ID_LIST PciChipset_t RHDPCIchipsets[]
# define LIST_END { 0,  0}

const PCI_ID_LIST = {
    RHD_DEVICE_MATCH(  0x7100, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x7101, RHD_M58   ), /* Mobility Radeon X1800 XT */
    RHD_DEVICE_MATCH(  0x7102, RHD_M58   ), /* Mobility Radeon X1800 */
    RHD_DEVICE_MATCH(  0x7103, RHD_M58   ), /* Mobility FireGL V7200 */
    RHD_DEVICE_MATCH(  0x7104, RHD_R520  ), /* FireGL V7200 */
    RHD_DEVICE_MATCH(  0x7105, RHD_R520  ), /* FireGL V5300 */
    RHD_DEVICE_MATCH(  0x7106, RHD_M58   ), /* Mobility FireGL V7100 */
    RHD_DEVICE_MATCH(  0x7108, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x7109, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x710A, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x710B, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x710C, RHD_R520  ), /* Radeon X1800 */
    RHD_DEVICE_MATCH(  0x710E, RHD_R520  ), /* FireGL V7300 */
    RHD_DEVICE_MATCH(  0x710F, RHD_R520  ), /* FireGL V7350 */
    RHD_DEVICE_MATCH(  0x7140, RHD_RV515 ), /* Radeon X1600/X1550 */
    RHD_DEVICE_MATCH(  0x7141, RHD_RV505 ), /* RV505 */
    RHD_DEVICE_MATCH(  0x7142, RHD_RV515 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7143, RHD_RV505 ), /* Radeon X1550 */
    RHD_DEVICE_MATCH(  0x7144, RHD_M54   ), /* M54-GL */
    RHD_DEVICE_MATCH(  0x7145, RHD_M54   ), /* Mobility Radeon X1400 */
    RHD_DEVICE_MATCH(  0x7146, RHD_RV515 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7147, RHD_RV505 ), /* Radeon X1550 64-bit */
    RHD_DEVICE_MATCH(  0x7149, RHD_M52   ), /* Mobility Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714A, RHD_M52   ), /* Mobility Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714B, RHD_M52   ), /* Mobility Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714C, RHD_M52   ), /* Mobility Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714D, RHD_RV515 ), /* Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714E, RHD_RV515 ), /* Radeon X1300 */
    RHD_DEVICE_MATCH(  0x714F, RHD_RV505 ), /* RV505 */
    RHD_DEVICE_MATCH(  0x7151, RHD_RV505 ), /* RV505 */
    RHD_DEVICE_MATCH(  0x7152, RHD_RV515 ), /* FireGL V3300 */
    RHD_DEVICE_MATCH(  0x7153, RHD_RV515 ), /* FireGL V3350 */
    RHD_DEVICE_MATCH(  0x715E, RHD_RV515 ), /* Radeon X1300 */
    RHD_DEVICE_MATCH(  0x715F, RHD_RV505 ), /* Radeon X1550 64-bit */
    RHD_DEVICE_MATCH(  0x7180, RHD_RV516 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7181, RHD_RV516 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x7183, RHD_RV516 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7186, RHD_M64   ), /* Mobility Radeon X1450 */
    RHD_DEVICE_MATCH(  0x7187, RHD_RV516 ), /* Radeon X1300/X1550 */
    RHD_DEVICE_MATCH(  0x7188, RHD_M64   ), /* Mobility Radeon X2300 */
    RHD_DEVICE_MATCH(  0x718A, RHD_M64   ), /* Mobility Radeon X2300 */
    RHD_DEVICE_MATCH(  0x718B, RHD_M62   ), /* Mobility Radeon X1350 */
    RHD_DEVICE_MATCH(  0x718C, RHD_M62   ), /* Mobility Radeon X1350 */
    RHD_DEVICE_MATCH(  0x718D, RHD_M64   ), /* Mobility Radeon X1450 */
    RHD_DEVICE_MATCH(  0x718F, RHD_RV516 ), /* Radeon X1300 */
    RHD_DEVICE_MATCH(  0x7193, RHD_RV516 ), /* Radeon X1550 */
    RHD_DEVICE_MATCH(  0x7196, RHD_M62   ), /* Mobility Radeon X1350 */
    RHD_DEVICE_MATCH(  0x719B, RHD_RV516 ), /* FireMV 2250 */
    RHD_DEVICE_MATCH(  0x719F, RHD_RV516 ), /* Radeon X1550 64-bit */
    RHD_DEVICE_MATCH(  0x71C0, RHD_RV530 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71C1, RHD_RV535 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x71C2, RHD_RV530 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71C3, RHD_RV535 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71C4, RHD_M56   ), /* Mobility FireGL V5200 */
    RHD_DEVICE_MATCH(  0x71C5, RHD_M56   ), /* Mobility Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71C6, RHD_RV530 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x71C7, RHD_RV535 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x71CD, RHD_RV530 ), /* Radeon X1600 */
    RHD_DEVICE_MATCH(  0x71CE, RHD_RV530 ), /* Radeon X1300 XT/X1600 Pro */
    RHD_DEVICE_MATCH(  0x71D2, RHD_RV530 ), /* FireGL V3400 */
    RHD_DEVICE_MATCH(  0x71D4, RHD_M66   ), /* Mobility FireGL V5250 */
    RHD_DEVICE_MATCH(  0x71D5, RHD_M66   ), /* Mobility Radeon X1700 */
    RHD_DEVICE_MATCH(  0x71D6, RHD_M66   ), /* Mobility Radeon X1700 XT */
    RHD_DEVICE_MATCH(  0x71DA, RHD_RV530 ), /* FireGL V5200 */
    RHD_DEVICE_MATCH(  0x71DE, RHD_M66   ), /* Mobility Radeon X1700 */
    RHD_DEVICE_MATCH(  0x7200, RHD_RV550 ), /*  Radeon X2300HD  */
    RHD_DEVICE_MATCH(  0x7210, RHD_M71   ), /* Mobility Radeon HD 2300 */
    RHD_DEVICE_MATCH(  0x7211, RHD_M71   ), /* Mobility Radeon HD 2300 */
    RHD_DEVICE_MATCH(  0x7240, RHD_R580  ), /* Radeon X1950 */
    RHD_DEVICE_MATCH(  0x7243, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7244, RHD_R580  ), /* Radeon X1950 */
    RHD_DEVICE_MATCH(  0x7245, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7246, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7247, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7248, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7249, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724A, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724B, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724C, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724D, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x724E, RHD_R580  ), /* AMD Stream Processor */
    RHD_DEVICE_MATCH(  0x724F, RHD_R580  ), /* Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7280, RHD_RV570 ), /* Radeon X1950 */
    RHD_DEVICE_MATCH(  0x7281, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x7283, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x7284, RHD_M68   ), /* Mobility Radeon X1900 */
    RHD_DEVICE_MATCH(  0x7287, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x7288, RHD_RV570 ), /* Radeon X1950 GT */
    RHD_DEVICE_MATCH(  0x7289, RHD_RV570 ), /* RV570 */
    RHD_DEVICE_MATCH(  0x728B, RHD_RV570 ), /* RV570 */
    RHD_DEVICE_MATCH(  0x728C, RHD_RV570 ), /* ATI FireGL V7400  */
    RHD_DEVICE_MATCH(  0x7290, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x7291, RHD_RV560 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x7293, RHD_RV560 ), /* Radeon X1650 */
    RHD_DEVICE_MATCH(  0x7297, RHD_RV560 ), /* RV560 */
    RHD_DEVICE_MATCH(  0x791E, RHD_RS690 ), /* Radeon X1200 */
    RHD_DEVICE_MATCH(  0x791F, RHD_RS690 ), /* Radeon X1200 */
    RHD_DEVICE_MATCH(  0x793F, RHD_RS600 ), /* Radeon Xpress 1200 */
    RHD_DEVICE_MATCH(  0x7941, RHD_RS600 ), /* Radeon Xpress 1200 */
    RHD_DEVICE_MATCH(  0x7942, RHD_RS600 ), /* Radeon Xpress 1200 (M) */
    RHD_DEVICE_MATCH(  0x796C, RHD_RS740 ), /* RS740 */
    RHD_DEVICE_MATCH(  0x796D, RHD_RS740 ), /* RS740M */
    RHD_DEVICE_MATCH(  0x796E, RHD_RS740 ), /* ATI Radeon 2100 RS740 */
    RHD_DEVICE_MATCH(  0x796F, RHD_RS740 ), /* RS740M */
    RHD_DEVICE_MATCH(  0x9400, RHD_R600  ), /* Radeon HD 2900 XT */
    RHD_DEVICE_MATCH(  0x9401, RHD_R600  ), /* Radeon HD 2900 XT */
    RHD_DEVICE_MATCH(  0x9402, RHD_R600  ), /* Radeon HD 2900 XT */
    RHD_DEVICE_MATCH(  0x9403, RHD_R600  ), /* Radeon HD 2900 Pro */
    RHD_DEVICE_MATCH(  0x9405, RHD_R600  ), /* Radeon HD 2900 GT */
    RHD_DEVICE_MATCH(  0x940A, RHD_R600  ), /* FireGL V8650 */
    RHD_DEVICE_MATCH(  0x940B, RHD_R600  ), /* FireGL V8600 */
    RHD_DEVICE_MATCH(  0x940F, RHD_R600  ), /* FireGL V7600 */
    RHD_DEVICE_MATCH(  0x9440, RHD_RV770 ), /* ATI Radeon 4800 Series  */
    RHD_DEVICE_MATCH(  0x9441, RHD_RV770 ), /* ATI Radeon 4870 X2  */
    RHD_DEVICE_MATCH(  0x9442, RHD_RV770 ), /* ATI Radeon 4800 Series  */
//    RHD_DEVICE_MATCH(  0x9443, RHD_R700  ), /* ATI Radeon 4800 Series  */
    RHD_DEVICE_MATCH(  0x9444, RHD_RV770 ), /* Everest ATI FirePro Graphics Accelerator  */
    RHD_DEVICE_MATCH(  0x9446, RHD_RV770 ), /* K2 ATI FirePro Graphics Accelerator  */
//    RHD_DEVICE_MATCH(  0x9447, RHD_R700 ), /* K2 ATI FirePro Graphics Accelerator  */
    RHD_DEVICE_MATCH(  0x944A, RHD_M98 ), /* M98  */
    RHD_DEVICE_MATCH(  0x944B, RHD_M98 ), /* M98  */
    RHD_DEVICE_MATCH(  0x944E, RHD_RV770 ), /* RV770  */
//    RHD_DEVICE_MATCH(  0x944F, RHD_R700 ), /* R700  */
    RHD_DEVICE_MATCH(  0x9456, RHD_RV770 ), /* Denali ATI FirePro Graphics Accelerator  */
    RHD_DEVICE_MATCH(  0x945A, RHD_M98 ), /* M98 */
    RHD_DEVICE_MATCH(  0x945B, RHD_M98 ), /* M98 */
    RHD_DEVICE_MATCH(  0x946A, RHD_M98 ), /* M98 */
    RHD_DEVICE_MATCH(  0x946B, RHD_M98 ), /* M98 */
    RHD_DEVICE_MATCH(  0x947A, RHD_M98 ), /* M98 */
    RHD_DEVICE_MATCH(  0x947B, RHD_M98 ), /* M96 */
    RHD_DEVICE_MATCH(  0x9480, RHD_M96 ), /* M98 */
    RHD_DEVICE_MATCH(  0x9487, RHD_RV730 ), /* RV730 */
    RHD_DEVICE_MATCH(  0x9488, RHD_M96 ), /* M96 */
    RHD_DEVICE_MATCH(  0x9489, RHD_M96 ), /* M96M GL */
    RHD_DEVICE_MATCH(  0x948F, RHD_RV730 ), /* RV730 */
    RHD_DEVICE_MATCH(  0x9487, RHD_RV730 ), /* RV730 */
    RHD_DEVICE_MATCH(  0x9490, RHD_RV730 ), /* RV730 */
    RHD_DEVICE_MATCH(  0x9498, RHD_RV730 ), /* RV730 */
    RHD_DEVICE_MATCH(  0x949E, RHD_RV730 ), /* RV730 */
    RHD_DEVICE_MATCH(  0x949F, RHD_RV730 ), /* RV730 */
    RHD_DEVICE_MATCH(  0x94C0, RHD_RV610 ), /* RV610 */
    RHD_DEVICE_MATCH(  0x94C1, RHD_RV610 ), /* Radeon HD 2400 XT */
    RHD_DEVICE_MATCH(  0x94C3, RHD_RV610 ), /* Radeon HD 2400 Pro */
    RHD_DEVICE_MATCH(  0x94C4, RHD_RV610 ), /* ATI Radeon HD 2400 PRO AGP */
    RHD_DEVICE_MATCH(  0x94C5, RHD_RV610 ), /* FireGL V4000 */
    RHD_DEVICE_MATCH(  0x94C6, RHD_RV610 ), /* RV610 */
    RHD_DEVICE_MATCH(  0x94C7, RHD_RV610 ), /* ATI Radeon HD 2350 */
    RHD_DEVICE_MATCH(  0x94C8, RHD_M74   ), /* Mobility Radeon HD 2400 XT */
    RHD_DEVICE_MATCH(  0x94C9, RHD_M72   ), /* Mobility Radeon HD 2400 */
    RHD_DEVICE_MATCH(  0x94CB, RHD_M72   ), /* ATI RADEON E2400 */
    RHD_DEVICE_MATCH(  0x94CC, RHD_RV610 ), /* ATI Radeon HD 2400 */
    RHD_DEVICE_MATCH(  0x9500, RHD_RV670 ), /* RV670 */
    RHD_DEVICE_MATCH(  0x9501, RHD_RV670 ), /* ATI Radeon HD3870 */
    RHD_DEVICE_MATCH(  0x9504, RHD_M88   ), /*  ATI Mobility Radeon HD 3850 */
    RHD_DEVICE_MATCH(  0x9505, RHD_RV670 ), /* ATI Radeon HD3850 */
    RHD_DEVICE_MATCH(  0x9506, RHD_M88   ), /*  ATI Mobility Radeon HD 3850 X2 */
    RHD_DEVICE_MATCH(  0x9507, RHD_RV670 ), /* RV670 */
    RHD_DEVICE_MATCH(  0x9508, RHD_M88   ), /*  ATI Mobility Radeon HD 3870 */
    RHD_DEVICE_MATCH(  0x9509, RHD_M88   ), /*  ATI Mobility Radeon HD 3870 X2 */
    RHD_DEVICE_MATCH(  0x950F, RHD_R680  ), /* ATI Radeon HD3870 X2 */
    RHD_DEVICE_MATCH(  0x9511, RHD_RV670 ), /* ATI FireGL V7700 */
    RHD_DEVICE_MATCH(  0x9515, RHD_RV670 ), /* ATI Radeon HD 3850 AGP */
    RHD_DEVICE_MATCH(  0x9517, RHD_RV670 ), /* ATI Radeon HD 3960 */
    RHD_DEVICE_MATCH(  0x9519, RHD_RV670 ), /* FireStream 9170 */
    RHD_DEVICE_MATCH(  0x9540, RHD_RV710 ), /*   */
    RHD_DEVICE_MATCH(  0x9541, RHD_RV710 ), /*   */
    RHD_DEVICE_MATCH(  0x9542, RHD_RV710 ), /*   */
    RHD_DEVICE_MATCH(  0x954E, RHD_RV710 ), /*   */
    RHD_DEVICE_MATCH(  0x954f, RHD_RV710 ), /*   */
    RHD_DEVICE_MATCH(  0x9580, RHD_RV630 ), /* RV630 */
    RHD_DEVICE_MATCH(  0x9581, RHD_M76   ), /* Mobility Radeon HD 2600 */
    RHD_DEVICE_MATCH(  0x9583, RHD_M76   ), /* Mobility Radeon HD 2600 XT */
    RHD_DEVICE_MATCH(  0x9586, RHD_RV630 ), /* ATI Radeon HD 2600 XT AGP */
    RHD_DEVICE_MATCH(  0x9587, RHD_RV630 ), /* ATI Radeon HD 2600 Pro AGP */
    RHD_DEVICE_MATCH(  0x9588, RHD_RV630 ), /* Radeon HD 2600 XT */
    RHD_DEVICE_MATCH(  0x9589, RHD_RV630 ), /* Radeon HD 2600 Pro */
    RHD_DEVICE_MATCH(  0x958A, RHD_RV630 ), /* Gemini RV630 */
    RHD_DEVICE_MATCH(  0x958B, RHD_M76   ), /* Gemini ATI Mobility Radeon HD 2600 XT */
    RHD_DEVICE_MATCH(  0x958C, RHD_RV630 ), /* FireGL V5600 */
    RHD_DEVICE_MATCH(  0x958D, RHD_RV630 ), /* FireGL V3600 */
    RHD_DEVICE_MATCH(  0x958E, RHD_RV630 ), /* ATI Radeon HD 2600 LE */
    RHD_DEVICE_MATCH(  0x958F, RHD_M76   ), /* ATI Mobility FireGL Graphics Processor */
    RHD_DEVICE_MATCH(  0x9590, RHD_RV635 ), /* ATI Radeon HD 3600 Series */
    RHD_DEVICE_MATCH(  0x9591, RHD_M86   ), /* Mobility Radeon HD 3650 */
    RHD_DEVICE_MATCH(  0x9593, RHD_M86   ), /* Mobility Radeon HD 3670 */
    RHD_DEVICE_MATCH(  0x9595, RHD_M86   ), /* Mobility FireGL V5700 */
    RHD_DEVICE_MATCH(  0x9596, RHD_RV635 ), /* ATI Radeon HD 3650 AGP */
    RHD_DEVICE_MATCH(  0x9597, RHD_RV635 ), /* ATI Radeon HD 3600 Series */
    RHD_DEVICE_MATCH(  0x9598, RHD_RV635 ), /* ATI Radeon HD 3670 */
    RHD_DEVICE_MATCH(  0x9599, RHD_RV635 ), /* ATI Radeon HD 3600 Series */
    RHD_DEVICE_MATCH(  0x959B, RHD_M86   ), /* Mobility FireGL Graphics Processor */
    RHD_DEVICE_MATCH(  0x95C0, RHD_RV620 ), /* ATI Radeon HD 3470 */
    RHD_DEVICE_MATCH(  0x95C2, RHD_M82   ), /* ATI Mobility Radeon HD 3430 (M82) */
    RHD_DEVICE_MATCH(  0x95C4, RHD_M82   ), /* Mobility Radeon HD 3400 Series (M82)  */
    RHD_DEVICE_MATCH(  0x95C5, RHD_RV620 ), /* ATI Radeon HD 3450 */
    RHD_DEVICE_MATCH(  0x95C6, RHD_RV620 ), /* ATI Radeon HD 3450 */
    RHD_DEVICE_MATCH(  0x95C7, RHD_RV620 ), /* ATI Radeon HD 3430 */
    RHD_DEVICE_MATCH(  0x95CC, RHD_RV620 ), /* Fire PRO Professional Graphics ASIC  */
    RHD_DEVICE_MATCH(  0x95CD, RHD_RV620 ), /* ATI FireMV 2450  */
    RHD_DEVICE_MATCH(  0x95CE, RHD_RV620 ), /* ATI FireMV 2260  */
    RHD_DEVICE_MATCH(  0x95CF, RHD_RV620 ), /* ATI FireMV 2260  */
    RHD_DEVICE_MATCH(  0x9610, RHD_RS780 ), /* ATI Radeon HD 3200 Graphics */
    RHD_DEVICE_MATCH(  0x9611, RHD_RS780 ), /* ATI Radeon 3100 Graphics */
    RHD_DEVICE_MATCH(  0x9612, RHD_RS780 ), /*  ATI Radeon HD 3200 Graphics  */
    RHD_DEVICE_MATCH(  0x9613, RHD_RS780 ), /* ATI Radeon 3100 Graphics   */
    RHD_DEVICE_MATCH(  0x9614, RHD_RS780 ), /* ATI Radeon HD 3300 Graphics  */
    LIST_END
};

static enum RHD_CHIPSETS rhdIGPChipsetList[] = {
    RHD_RS690,
    RHD_RS690,
    RHD_RS690,
    RHD_RS780,
    RHD_UNKNOWN /* end marker */
};

/*
 *
 */
void
RHDIdentify(int flags)
{

}

/*
 *
 */
Bool
RHDIsIGP(enum RHD_CHIPSETS chipset)
{
    int i = 0;
    while (rhdIGPChipsetList[i] != RHD_UNKNOWN) {
	if (chipset == (rhdIGPChipsetList[i]))
	    return TRUE;
	i++;
    }
    return FALSE;
}

/*
 * Some macros to help us make connector tables less messy.
 * There are, after all, a limited number of possibilities at the moment.
 */
#define ID_CONNECTORINFO_EMPTY \
     { {RHD_CONNECTOR_NONE, "NULL", RHD_DDC_NONE, RHD_HPD_NONE, \
       { RHD_OUTPUT_NONE, RHD_OUTPUT_NONE}}}

#ifdef ATOM_BIOS
# define DEVINFO_EMPTY   { { atomNone, atomNone } }
#endif

/* Radeon RV610 0x94C3 0x0000 0x0000 */
#define VGA_B1_TV_B_DVI_AA00 \
{{ RHD_CONNECTOR_DVI_SINGLE, "VGA CRT2", RHD_DDC_1, RHD_HPD_NONE, \
	{ RHD_OUTPUT_NONE, RHD_OUTPUT_DACB }},			   \
    {RHD_CONNECTOR_TV, "7PIN_DIN TV1 CV", RHD_DDC_0, RHD_HPD_NONE, \
	{ RHD_OUTPUT_DACB, RHD_OUTPUT_NONE }},			   \
    {RHD_CONNECTOR_DVI_SINGLE, "SINGLE_LINK_DVI CRT1 DFP2", RHD_DDC_0, RHD_HPD_0, \
	{RHD_OUTPUT_LVTMA, RHD_OUTPUT_DACA }}}


/* Radeon X1300 0x7187:0x1545:0x1930 */
#define VGA_A0_TV_B_DVI_B11 \
    {  { RHD_CONNECTOR_VGA, "VGA CRT1", RHD_DDC_0, RHD_HPD_NONE,    \
	{ RHD_OUTPUT_DACA, RHD_OUTPUT_NONE }}, 			    \
    { RHD_CONNECTOR_TV, "SVIDEO TV1", RHD_DDC_NONE, RHD_HPD_NONE,   \
	{ RHD_OUTPUT_DACB, RHD_OUTPUT_NONE }},			    \
    { RHD_CONNECTOR_DVI, "DVI-D DFP3", RHD_DDC_1, RHD_HPD_1,	    \
	{ RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE }}}

/* Sapphire X1550 reports 2x DVI-I but has only 1 VGA and 1 DVI */
#define VGA_A0_DVI_BB11 \
  { {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_1, RHD_HPD_1, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_LVTMA}}}

/* 0x7249:0x1043:0x0168 */
#define DVI_AB10_DVI_A01 \
  { { RHD_CONNECTOR_DVI, "DVI-I DFP1 CRT2", RHD_DDC_1, RHD_HPD_0,	\
         { RHD_OUTPUT_TMDSA, RHD_OUTPUT_DACB }}, \
      { RHD_CONNECTOR_DVI, "DVI-I DFP2", RHD_DDC_0, RHD_HPD_1, \
         { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE }}, \
      { RHD_CONNECTOR_TV, "SVIDEO TV1", RHD_DDC_NONE, RHD_HPD_NONE, \
         { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE }}}

#define VISIONTEK_C1550 \
    { {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE,  \
	    { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE } },	   \
      {RHD_CONNECTOR_TV, "SVIDEO", RHD_DDC_NONE, RHD_HPD_NONE, \
	  { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE } },	   \
      {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_1, RHD_HPD_0, \
	  { RHD_OUTPUT_LVTMA, RHD_OUTPUT_DACB } } }

/* MacBook Pro provides a weird atombios connector table. */
#define ID_CONNECTORINFO_MACBOOKPRO \
 { {RHD_CONNECTOR_PANEL, "Panel", RHD_DDC_2, RHD_HPD_NONE, \
       { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE}}, \
   {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_0, RHD_HPD_0, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_TMDSA}}}

#ifdef ATOM_BIOS
# define DEVINFO_MACBOOKPRO \
    { { atomLCD1, atomNone }, { atomCRT2, atomDFP1 } }
#endif

/* GeCube HD 2400PRO AGP (GC-RX24PGA2-D3) specifies 2 DVI again.*/
#define BROKEN_VGA_B1_DVI_AB00 \
 { {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_0, RHD_HPD_0, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_LVTMA}}, \
   {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_1, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE}}}

/* Fujitsu Siemens Amilo PI1536 has no HPD on its DVI connector. */
#define PANEL_B_DVI_AA1 \
 { {RHD_CONNECTOR_PANEL, "Panel", RHD_DDC_NONE, RHD_HPD_NONE, \
       { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE}}, \
   {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_0, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_TMDSA}}}

/* Sapphire Radeon HD 2600 PRO AGP reports VGA output as DVI */
#define DVI_BA10_TV_B0_VGA_A0 \
 { { RHD_CONNECTOR_DVI, "DUAL_LINK_DVI_I", RHD_DDC_1, RHD_HPD_0, \
	{ RHD_OUTPUT_TMDSA, RHD_OUTPUT_DACB }}, \
    { RHD_CONNECTOR_TV, "7PIN_DIN TV1 CV", RHD_DDC_0, RHD_HPD_NONE, \
	{ RHD_OUTPUT_DACB, RHD_OUTPUT_NONE }}, \
    { RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE, \
	{ RHD_OUTPUT_NONE, RHD_OUTPUT_DACA }}}

/* MSI RX2600PRO-T2D512Z/D2 */
#define DVI_BA12_TV_B0_DVI_AB01 \
 { { RHD_CONNECTOR_DVI, "DUAL_LINK_DVI_I DFP1 CRT2", RHD_DDC_1, RHD_HPD_2, \
	 { RHD_OUTPUT_TMDSA, RHD_OUTPUT_DACB }}, \
   { RHD_CONNECTOR_TV, "7PIN_DIN TV1 CV", RHD_DDC_NONE, RHD_HPD_NONE, \
	 { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE }}, \
   { RHD_CONNECTOR_DVI, "DUAL_LINK_DVI_I CRT1 DFP2", RHD_DDC_0, RHD_HPD_1, \
	 { RHD_OUTPUT_LVTMA, RHD_OUTPUT_DACA }}}

#if defined(USE_ID_CONNECTORS) || !defined(ATOM_BIOS)

#define	VGA_A0_TVB_DVI_BB12				       \
  { { RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE, \
        { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE }}, \
    { RHD_CONNECTOR_TV, "SVIDEO", RHD_DDC_NONE, RHD_HPD_NONE, \
        { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE }}, \
    { RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_1, RHD_HPD_2, \
        { RHD_OUTPUT_DACB, RHD_OUTPUT_LVTMA }}}

#define VGA_A0_DVI_BA10 \
  { {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_1, RHD_HPD_0, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_TMDSA}}}

#define VGA_A0_DVI_BB10 \
  { {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_1, RHD_HPD_0, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_LVTMA}}}

#define VGA_B1_DVI_AA00 \
  { {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_1, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_0, RHD_HPD_0, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_TMDSA}}}

#define VGA_B1_DVI_AB01 \
  { {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_1, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_0, RHD_HPD_1, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_LVTMA}}}

#define VGA_B1_DVI_AB00 \
  { {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_1, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_DVI, "DVI-I", RHD_DDC_0, RHD_HPD_0, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_LVTMA}}}

#define DVI_AA00_DVI_BB11 \
  { {RHD_CONNECTOR_DVI, "DVI-I 1", RHD_DDC_0, RHD_HPD_0, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_TMDSA}}, \
    {RHD_CONNECTOR_DVI, "DVI-I 2", RHD_DDC_1, RHD_HPD_1, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_LVTMA}}}

#define DVI_BA10_DVI_AB01 \
  { {RHD_CONNECTOR_DVI, "DVI-I 1", RHD_DDC_1, RHD_HPD_0, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_TMDSA}}, \
    {RHD_CONNECTOR_DVI, "DVI-I 2", RHD_DDC_0, RHD_HPD_1, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_LVTMA}}}

#define DVI_BB11_DVI_AA00 \
  { {RHD_CONNECTOR_DVI, "DVI-I 1", RHD_DDC_1, RHD_HPD_1, \
       { RHD_OUTPUT_DACB, RHD_OUTPUT_LVTMA}}, \
    {RHD_CONNECTOR_DVI, "DVI-I 2", RHD_DDC_0, RHD_HPD_0, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_TMDSA}}}

#define PANEL_B_VGA_A0 \
  { {RHD_CONNECTOR_PANEL, "Panel", RHD_DDC_NONE, RHD_HPD_NONE, \
       { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE}}}

#define PANEL_B1_VGA_A0 \
  { {RHD_CONNECTOR_PANEL, "Panel", RHD_DDC_1, RHD_HPD_NONE, \
       { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE}}}

#define PANEL_B1_VGA_A2 \
  { {RHD_CONNECTOR_PANEL, "Panel", RHD_DDC_1, RHD_HPD_NONE, \
       { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_2, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE}}}

#define PANEL_B2_VGA_A0 \
  { {RHD_CONNECTOR_PANEL, "Panel", RHD_DDC_2, RHD_HPD_NONE, \
       { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE}}}

#define PANEL_B2_VGA_A0_DVI_A10 \
  { {RHD_CONNECTOR_PANEL, "Panel", RHD_DDC_2, RHD_HPD_NONE, \
       { RHD_OUTPUT_LVTMA, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_VGA, "VGA", RHD_DDC_0, RHD_HPD_NONE, \
       { RHD_OUTPUT_DACA, RHD_OUTPUT_NONE}}, \
    {RHD_CONNECTOR_DVI, "DVI-D", RHD_DDC_1, RHD_HPD_0, \
       { RHD_OUTPUT_TMDSA, RHD_OUTPUT_NONE}}}

#else /* if !defined(USE_ID_CONNECTORS) && defined(ATOM_BIOS) */

#define VGA_A0_TVB_DVI_BB12     ID_CONNECTORINFO_EMPTY
#define VGA_A0_DVI_BA10         ID_CONNECTORINFO_EMPTY
#define VGA_A0_DVI_BB10         ID_CONNECTORINFO_EMPTY
#define VGA_B1_DVI_AA00         ID_CONNECTORINFO_EMPTY
#define VGA_B1_DVI_AB01         ID_CONNECTORINFO_EMPTY
#define VGA_B1_DVI_AB00         ID_CONNECTORINFO_EMPTY
#define DVI_AA00_DVI_BB11       ID_CONNECTORINFO_EMPTY
#define DVI_BA10_DVI_AB01       ID_CONNECTORINFO_EMPTY
#define DVI_BB11_DVI_AA00       ID_CONNECTORINFO_EMPTY
#define PANEL_B_VGA_A0          ID_CONNECTORINFO_EMPTY
#define PANEL_B1_VGA_A0         ID_CONNECTORINFO_EMPTY
#define PANEL_B1_VGA_A2         ID_CONNECTORINFO_EMPTY
#define PANEL_B2_VGA_A0         ID_CONNECTORINFO_EMPTY
#define PANEL_B2_VGA_A0_DVI_A10 ID_CONNECTORINFO_EMPTY

#endif /* if defined(USE_ID_CONNECTORS) || !defined(ATOM_BIOS) */

/*
 * List of pci subsystem / card ids.
 *
 * Used for:
 * - printing card name.
 * - connector mapping.
 *
 */
static struct rhdCard
rhdCards[] =
{
    /* 0x7100 : R520 : Radeon X1800 */
    { 0x7100, 0x1002, 0x0B12, "Powercolor X1800XT", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    /* 0x7101 : M58 : Mobility Radeon X1800 XT */
    /* 0x7102 : M58 : Mobility Radeon X1800 */
    /* 0x7103 : M58 : Mobility FireGL V7200 */
    /* 0x7104 : R520 : FireGL V7200 */
    { 0x7104, 0x1002, 0x0B32, "ATI FireGL V7200 RH", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    /* 0x7105 : R520 : FireGL V5300 */
    /* 0x7106 : M58 : Mobility FireGL V7100 */
    /* 0x7108 : R520 : Radeon X1800 */
    /* 0x7109 : R520 : Radeon X1800 */
    /* 0x710A : R520 : Radeon X1800 */
    /* 0x710B : R520 : Radeon X1800 */
    /* 0x710C : R520 : Radeon X1800 */
    /* 0x710E : R520 : FireGL V7300 */
    /* 0x710F : R520 : FireGL V7350 */
    /* 0x7140 : RV515 : Radeon X1600 */
    { 0x7140, 0x1787, 0x3000, "PowerColor X1550", RHD_CARD_FLAG_HPDSWAP, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x7141 : RV505 : RV505 */
    /* 0x7142 : RV515 : Radeon X1300/X1550 */
    /* 0x7143 : RV505 : Radeon X1550 */
    /* 0x7144 : M54 : M54-GL */
    /* 0x7145 : M54 : Mobility Radeon X1400 */
    { 0x7145, 0x1028, 0x2002, "Dell Inspiron 9400", RHD_CARD_FLAG_NONE, PANEL_B2_VGA_A0_DVI_A10, DEVINFO_EMPTY },
    { 0x7145, 0x1028, 0x2003, "Dell Inspiron 6400", RHD_CARD_FLAG_NONE, PANEL_B_VGA_A0, DEVINFO_EMPTY },
    { 0x7145, 0x1179, 0xFF10, "Toshiba Satellite A100-773", RHD_CARD_FLAG_NONE, PANEL_B1_VGA_A2, DEVINFO_EMPTY },
    { 0x7145, 0x1297, 0x3058, "M54P X1440", RHD_CARD_FLAG_HPDOFF, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    { 0x7145, 0x1734, 0x10B0, "Fujitsu Siemens Amilo PI1536", RHD_CARD_FLAG_NONE, PANEL_B_DVI_AA1, DEVINFO_EMPTY },
    { 0x7145, 0x17AA, 0x2006, "Lenovo Thinkpad T60 (2007)", RHD_CARD_FLAG_NONE, PANEL_B2_VGA_A0_DVI_A10, DEVINFO_EMPTY },
    { 0x7145, 0x17AA, 0x202A, "Lenovo Thinkpad Z61m", RHD_CARD_FLAG_NONE, PANEL_B2_VGA_A0, DEVINFO_EMPTY },
    /* 0x7146 : RV515 : Radeon X1300/X1550 */
    { 0x7146, 0x174B, 0x0470, "Sapphire X1300", RHD_CARD_FLAG_NONE, VGA_B1_DVI_AB01, DEVINFO_EMPTY },
    { 0x7146, 0x174B, 0x0920, "Sapphire X1300", RHD_CARD_FLAG_HPDSWAP, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    { 0x7146, 0x1545, 0x2350, "Visiontek C1550", RHD_CARD_FLAG_NONE,  VISIONTEK_C1550, DEVINFO_EMPTY },
    /* 0x7147 : RV505 : Radeon X1550 64-bit */
    { 0x7147, 0x174B, 0x0840, "Sapphire X1550", RHD_CARD_FLAG_HPDSWAP, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x7149 : M52 : Mobility Radeon X1300 */
    { 0x7149, 0x1028, 0x2003, "Dell Inspiron E1505", RHD_CARD_FLAG_NONE, PANEL_B_VGA_A0, DEVINFO_EMPTY },
    { 0x7149, 0x17AA, 0x2005, "Lenovo Thinkpad T60 (2008)", RHD_CARD_FLAG_NONE, PANEL_B2_VGA_A0_DVI_A10, DEVINFO_EMPTY },
    /* 0x714A : M52 : Mobility Radeon X1300 */
    /* 0x714B : M52 : Mobility Radeon X1300 */
    /* 0x714C : M52 : Mobility Radeon X1300 */
    /* 0x714D : RV515 : Radeon X1300 */
    /* 0x714E : RV515 : Radeon X1300 */
    /* 0x714F : RV505 : RV505 */
    /* 0x7151 : RV505 : RV505 */
    /* 0x7152 : RV515 : FireGL V3300 */
    { 0x7152, 0x1002, 0x0B02, "ATI FireGL V3300", RHD_CARD_FLAG_NONE, DVI_BB11_DVI_AA00, DEVINFO_EMPTY },
    /* 0x7153 : RV515 : FireGL V3350 */
    /* 0x715E : RV515 : Radeon X1300 */
    /* 0x715F : RV505 : Radeon X1550 64-bit */
    /* 0x7180 : RV516 : Radeon X1300/X1550 */
    /* 0x7181 : RV516 : Radeon X1600 */
    /* 0x7183 : RV516 : Radeon X1300/X1550 */
    { 0x7183, 0x1028, 0x0D02, "Dell ATI Radeon X1300", RHD_CARD_FLAG_DMS59, DVI_AA00_DVI_BB11, DEVINFO_EMPTY },
    { 0x7183, 0x1092, 0x3000, "RX155PCI", RHD_CARD_FLAG_NONE, VGA_A0_TVB_DVI_BB12, DEVINFO_EMPTY },
    /* 0x7186 : M64 : Mobility Radeon X1450 */
    /* 0x7187 : RV516 : Radeon X1300/X1550 */
    { 0x7187, 0x174B, 0x3000, "RV516 : Radeon X1300/X1550", RHD_CARD_FLAG_HPDSWAP, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    { 0x7187, 0x1458, 0x215C, "RV516 : Radeon X1300/X1550", RHD_CARD_FLAG_DMS59,  ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    { 0x7187, 0x1545, 0x1930, "RV516 : Radeon X1300", RHD_CARD_FLAG_NONE, VGA_A0_TV_B_DVI_B11, DEVINFO_EMPTY },
    /* 0x7188 : M64 : Mobility Radeon X2300 */
    /* 0x718A : M64 : Mobility Radeon X2300 */
    /* 0x718B : M62 : Mobility Radeon X1350 */
    /* 0x718C : M62 : Mobility Radeon X1350 */
    /* 0x718D : M64 : Mobility Radeon X1450 */
    /* 0x718F : RV516 : Radeon X1300 */
    /* 0x7193 : RV516 : Radeon X1550 */
    /* 0x7196 : M62 : Mobility Radeon X1350 */
    /* 0x719B : RV516 : FireMV 2250 */
    /* 0x719F : RV516 : Radeon X1550 64-bit */
    /* 0x71C0 : RV530 : Radeon X1600 */
    /* 0x71C1 : RV535 : Radeon X1650 */
    { 0x71C1, 0x174B, 0x0840, "Sapphire X1650 Pro", RHD_CARD_FLAG_NONE, DVI_AA00_DVI_BB11, DEVINFO_EMPTY },
    /* 0x71C2 : RV530 : Radeon X1600 */
    { 0x71C2, 0x1458, 0x2146, "Gigabyte GV-RX16P256DE-RH", RHD_CARD_FLAG_HPDSWAP, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    { 0x71C2, 0x17EE, 0x71C0, "Connect3D Radeon X1600 Pro", RHD_CARD_FLAG_NONE, VGA_B1_DVI_AA00, DEVINFO_EMPTY },
    /* 0x71C3 : RV535 : Radeon X1600 */
    /* 0x71C4 : M56 : Mobility FireGL V5200 */
    { 0x71C4, 0x17AA, 0x2007, "Lenovo Thinkpad T60p V5200", RHD_CARD_FLAG_HPDOFF, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x71C5 : M56 : Mobility Radeon X1600 */
    { 0x71C5, 0x103C, 0x30A3, "HP/Compaq nc8430", RHD_CARD_FLAG_NONE, PANEL_B1_VGA_A0, DEVINFO_EMPTY },
    { 0x71C5, 0x103C, 0x30B4, "HP/Compaq nw8440", RHD_CARD_FLAG_NONE, PANEL_B1_VGA_A0, DEVINFO_EMPTY },
    { 0x71C5, 0x1043, 0x10B2, "Asus W3J/Z96", RHD_CARD_FLAG_NONE, PANEL_B_VGA_A0, DEVINFO_EMPTY },
    { 0x71C5, 0x106B, 0x0080, "Macbook Pro", RHD_CARD_FLAG_NONE, ID_CONNECTORINFO_MACBOOKPRO, DEVINFO_MACBOOKPRO },
    { 0x71C5, 0x1179, 0xFF10, "Toshiba Satellite A100-237", RHD_CARD_FLAG_NONE, PANEL_B1_VGA_A2, DEVINFO_EMPTY },
    /* 0x71C6 : RV530 : Radeon X1650 */
    { 0x71C6, 0x174B, 0x0850, "Sapphire X1650 Pro AGP", RHD_CARD_FLAG_NONE, VGA_A0_DVI_BA10, DEVINFO_EMPTY },
    { 0x71C6, 0x1462, 0x0400, "MSI RX1650 Pro", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    /* 0x71C7 : RV535 : Radeon X1650 */
    { 0x71C7, 0x1043, 0x01B6, "Asus EAX1650 Silent", RHD_CARD_FLAG_NONE, VGA_A0_DVI_BB10, DEVINFO_EMPTY },
    { 0x71C7, 0x1787, 0x2227, "Diamond Viper X1650 Pro", RHD_CARD_FLAG_HPDSWAP, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x71CD : RV530 : Radeon X1600 */
    { 0x71CD, 0x174B, 0x0840, "PCP X1600 400M/500E", RHD_CARD_FLAG_HPDSWAP, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x71CE : RV530 : Radeon X1300 XT/X1600 Pro */
    { 0x71CE, 0x18BC, 0x2770, "Radeon X1300 XT/X1600 Pro", RHD_CARD_FLAG_HPDOFF, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x71D2 : RV530 : FireGL V3400 */
    { 0x71D2, 0x1002, 0x2B02, "ATI FireGL V3400", RHD_CARD_FLAG_NONE, DVI_BB11_DVI_AA00, DEVINFO_EMPTY },
    /* 0x71D4 : M66 : Mobility FireGL V5250 */
    { 0x71D4, 0x17AA, 0x20A4, "Lenovo Thinkpad T60p V5250", RHD_CARD_FLAG_HPDOFF, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x71D5 : M66 : Mobility Radeon X1700 */
    /* 0x71D6 : M66 : Mobility Radeon X1700 XT */
    /* 0x71DA : RV530 : FireGL V5200 */
    /* 0x71DE : M66 : Mobility Radeon X1700 */
    /* 0x7200 : RV550 : Radeon X2300HD */
    /* 0x7210 : M71 : Mobility Radeon HD 2300 */
    /* 0x7211 : M71 : Mobility Radeon HD 2300 */
    /* 0x7240 : R580 : Radeon X1950 */
    /* 0x7243 : R580 : Radeon X1900 */
    /* 0x7244 : R580 : Radeon X1950 */
    /* 0x7245 : R580 : Radeon X1900 */
    /* 0x7246 : R580 : Radeon X1900 */
    /* 0x7247 : R580 : Radeon X1900 */
    /* 0x7248 : R580 : Radeon X1900 */
    /* 0x7249 : R580 : Radeon X1900 */
    { 0x7249, 0x1002, 0x0B12, "ATI Radeon X1900 XTX", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    /* { 0x7249, 0x1043, 0x016B, "ATI Radeon X1900 XTX", RHD_CARD_FLAG_NONE, DVI_AB10_DVI_A01, DEVINFO_EMPTY }, */
    /* 0x724A : R580 : Radeon X1900 */
    /* 0x724B : R580 : Radeon X1900 */
    { 0x724B, 0x1002, 0x0B12, "Sapphire Radeon X1900 GT", RHD_CARD_FLAG_NONE, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x724C : R580 : Radeon X1900 */
    /* 0x724D : R580 : Radeon X1900 */
    /* 0x724E : R580 : AMD Stream Processor */
    /* 0x724F : R580 : Radeon X1900 */
    /* 0x7280 : RV570 : Radeon X1950 */
    { 0x7280, 0x174B, 0xE190, "Sapphire X1950 Pro", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    { 0x7280, 0x18BC, 0x2870, "GeCube X1950 Pro", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    /* 0x7281 : RV560 : RV560 */
    /* 0x7283 : RV560 : RV560 */
    /* 0x7284 : M68 : Mobility Radeon X1900 */
    /* 0x7287 : RV560 : RV560 */
    /* 0x7288 : RV570 : Radeon X1950 GT */
    { 0x7288, 0x174B, 0xE190, "Sapphire X1950 GT", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    /* 0x7289 : RV570 : RV570 */
    /* 0x728B : RV570 : RV570 */
    /* 0x728C : RV570 : ATI FireGL V7400 */
    /* 0x7290 : RV560 : RV560 */
    /* 0x7291 : RV560 : Radeon X1650 */
    /* 0x7293 : RV560 : Radeon X1650 */
    /* 0x7297 : RV560 : RV560 */
    /* 0x791E : RS690 : Radeon X1200 */
    { 0x791E, 0x1043, 0x826D, "Asus M2A-VM", RHD_CARD_FLAG_NONE, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x791F : RS690 : Radeon X1200 */
    { 0x791F, 0x103C, 0x30C2, "HP/Compaq 6715b", RHD_CARD_FLAG_NONE, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    /* 0x793F : RS600 : Radeon Xpress 1200 */
    /* 0x7941 : RS600 : Radeon Xpress 1200 */
    /* 0x7942 : RS600 : Radeon Xpress 1200 (M) */
    /* 0x796C : RS740 : RS740 */
    /* 0x796D : RS740 : RS740M */
    /* 0x796E : RS740 : RS740 */
    /* 0x796F : RS740 : RS740M */
    /* 0x9400 : R600 : Radeon HD 2900 XT */
    { 0x9400, 0x1002, 0x3142, "Sapphire HD 2900 XT", RHD_CARD_FLAG_NONE, DVI_BB11_DVI_AA00, DEVINFO_EMPTY },
    /* 0x9401 : R600 : Radeon HD 2900 XT */
    /* 0x9402 : R600 : Radeon HD 2900 XT */
    /* 0x9403 : R600 : Radeon HD 2900 Pro */
    /* 0x9405 : R600 : ATI Radeon HD 2900 GT  */
    /* 0x940A : R600 : ATI FireGL V8650  */
    /* 0x940B : R600 : ATI FireGL V8600  */
    /* 0x940F : R600 : ATI FireGL V7600  */
    /* 0x94C0 : RV610 : RV610 */
    /* 0x94C1 : RV610 : Radeon HD 2400 XT */
    { 0x94C1, 0x1002, 0x0D02, "ATI Radeon HD 2400 XT", RHD_CARD_FLAG_DMS59, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    { 0x94C1, 0x1028, 0x0D02, "Dell Radeon HD 2400 XT", RHD_CARD_FLAG_DMS59, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    { 0x94C1, 0x174B, 0xE390, "Sapphire HD 2400 XT", RHD_CARD_FLAG_NONE, VGA_B1_DVI_AB00, DEVINFO_EMPTY },
    { 0x94C3, 0x0000, 0x0000, "ATI Radeon 2400 HD GENERIC", RHD_CARD_FLAG_NONE, VGA_B1_TV_B_DVI_AA00, DEVINFO_EMPTY },
    /* 0x94C3 : RV610 : Radeon HD 2400 Pro */
    { 0x94C3, 0x1545, 0x3210, "ATI Radeon 2400HD Pro", RHD_CARD_FLAG_HPDSWAP, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    { 0x94C3, 0x174B, 0xE370, "Sapphire HD 2400 Pro", RHD_CARD_FLAG_NONE, VGA_A0_DVI_BB10, DEVINFO_EMPTY },
    /* 0x94C4 : RV610 : ATI Radeon HD 2400 PRO AGP  */
    { 0x94C4, 0x18BC, 0x0028, "GeCube Radeon HD 2400PRO AGP", RHD_CARD_FLAG_NONE, BROKEN_VGA_B1_DVI_AB00, DEVINFO_EMPTY },
    /* 0x94C5 : RV610 : ATI FireGL V4000  */
    /* 0x94C6 : RV610 : RV610  */
    /* 0x94C7 : RV610 : ATI Radeon HD 2350 */
    /* 0x94C8 : M74 : Mobility Radeon HD 2400 XT */
    /* 0x94C9 : M72 : Mobility Radeon HD 2400 */
    /* 0x94CB : M72 : ATI RADEON E2400 */
    /* 0x94CC : RV610 : RV610  */
    /* 0x9505 : RV670 : ATI Radeon HD 3850 */
    /* 0x9580 : RV630 : RV630 */
    /* 0x9581 : M76 : Mobility Radeon HD 2600 */
    /* 0x9583 : M76 : Mobility Radeon HD 2600 XT */
    /* 0x9586 : RV630 : ATI Radeon HD 2600 XT AGP */
    /* 0x9587 : RV630 : ATI Radeon HD 2600 Pro AGP */
    { 0x9587, 0x1002, 0x0028, "Sapphire Radeon HD 2600 PRO AGP", RHD_CARD_FLAG_NONE, DVI_BA10_TV_B0_VGA_A0, DEVINFO_EMPTY },
    { 0x9587, 0x1462, 0x0028, "MSI HD2600PRO AGP", RHD_CARD_FLAG_NONE, DVI_BA12_TV_B0_DVI_AB01, DEVINFO_EMPTY },
    /* 0x9588 : RV630 : Radeon HD 2600 XT */
    { 0x9588, 0x1002, 0x2542, "ATI Radeon HD 2600XT DDR4", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    { 0x9588, 0x1448, 0x216C, "Gigabyte HD 2600 XT 256MB DDR3", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    { 0x9588, 0x174B, 0x2E42, "Sapphire HD 2600 XT", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    /* 0x9589 : RV630 : Radeon HD 2600 Pro */
    { 0x9589, 0x174B, 0xE410, "Sapphire HD 2600 Pro", RHD_CARD_FLAG_NONE, DVI_BA10_DVI_AB01, DEVINFO_EMPTY },
    /* 0x958A : RV630 : Gemini RV630 */
    /* 0x958B : M76 : Gemini ATI Mobility Radeon HD 2600 XT */
    /* 0x958C : RV630 : ATI FireGL V5600  */
    /* 0x958D : RV630 : ATI FireGL V3600  */
    /* 0x958E : RV630 : ATI Radeon HD 2600 LE  */
    { 0x9610, 0x105B, 0x0E0F, "Foxconn A7GM-S (RS780)", RHD_CARD_FLAG_HPDOFF, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY },
    { 0, 0, 0, NULL, 0, ID_CONNECTORINFO_EMPTY, DEVINFO_EMPTY } /* KEEP THIS: End marker. */
};

/*
 *
 */

struct rhdCard *RHDCardIdentify(RHDPtr rhdPtr)
{
    unsigned int deviceID, subVendorID, subDeviceID;
    int i;

    deviceID = (unsigned int) rhdPtr->PciDeviceID;
    subVendorID = (unsigned int)rhdPtr->subvendor_id;
    subDeviceID = (unsigned int)rhdPtr->subdevice_id;

    for (i = 0; rhdCards[i].name; i++)
      if ((rhdCards[i].device == deviceID) &&
          (rhdCards[i].card_vendor == subVendorID) &&
          (rhdCards[i].card_device == subDeviceID))
        return rhdCards + i;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "Unknown card detected: 0x%04X:0x%04X:0x%04X.\n",
         deviceID, subVendorID, subDeviceID);
    return NULL;
}

#define USE_ATOMBIOS RHD_RV770
#define USE_ATOM_CRTC USE_ATOMBIOS
#define USE_ATOM_PLL USE_ATOMBIOS
#define USE_ATOM_OUTPUT USE_ATOMBIOS

/*
 *
 */
Bool
RHDUseAtom(RHDPtr rhdPtr, enum RHD_CHIPSETS *BlackList,
	   enum atomSubSystem subsys)
{
#ifdef ATOM_BIOS
    Bool FromSys = FALSE, ret = FALSE;
    CARD32 FromUser = 0;
    int i = 0;
    char *message = NULL;
    enum RHD_CHIPSETS AtomChip;
    int from = 0;

    switch (subsys) {
	case atomUsageCrtc:
	    AtomChip = USE_ATOM_CRTC;
	    message = "Crtcs";
	    FromUser = (rhdPtr->UseAtomFlags >> RHD_ATOMBIOS_CRTC) & 0x7;
	    break;
	case atomUsagePLL:
	    AtomChip = USE_ATOM_PLL;
	    message = "PLLs";
	    FromUser = (rhdPtr->UseAtomFlags >> RHD_ATOMBIOS_PLL) & 0x7;
	    break;
	case atomUsageOutput:
	    AtomChip = USE_ATOM_OUTPUT;
	    message = "Outputs";
	    FromUser = (rhdPtr->UseAtomFlags >> RHD_ATOMBIOS_OUTPUT) & 0x7;
	    break;
	case atomUsageAny:
	    AtomChip = min(USE_ATOM_OUTPUT,min(USE_ATOM_PLL, USE_ATOM_CRTC));
	    message = "All";
	    FromUser = ((rhdPtr->UseAtomFlags >> RHD_ATOMBIOS_OUTPUT)
		| (rhdPtr->UseAtomFlags >> RHD_ATOMBIOS_PLL)
			| (rhdPtr->UseAtomFlags >> RHD_ATOMBIOS_CRTC)) & 0x7;
	    break;
    }

    if (rhdPtr->ChipSet >= AtomChip)
	FromSys = TRUE;

    if (!FromSys && BlackList) {
	while (BlackList[i] != RHD_CHIP_END) {
	    if (BlackList[i++] == rhdPtr->ChipSet) {
		FromSys = TRUE;
	    }
	}
    }
    if (!FromSys) {
	if (rhdPtr->UseAtomBIOS.set) {
//        from = X_CONFIG;
	    ret = rhdPtr->UseAtomBIOS.val.bool;
	}
	if (FromUser & RHD_ATOMBIOS_ON)
	    ret = TRUE;
	if (FromUser & RHD_ATOMBIOS_OFF)
	    ret = FALSE;
    } else {
	ret = TRUE;
	if ((FromUser & RHD_ATOMBIOS_FORCE) && (FromUser & RHD_ATOMBIOS_OFF)) {
 //       from = X_CONFIG;
	    ret = FALSE;
	}
    }
    if (ret)
    xf86DrvMsg(rhdPtr->scrnIndex, from, "Using AtomBIOS for %s\n",
           message);

    return ret;
#else
    return 0;
#endif /* ATOM_BIOS */
}
