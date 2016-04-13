/***********************************************************************
 *
 *  avra - Assembler for the Atmel AVR microcontroller series
 *
 *  Copyright (C) 1998-2010 Jon Anders Haugum, Tobias Weber, Jerry Jacobs
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *
 *  Authors of avra can be reached at:
 *     email: jonah@omegav.ntnu.no, tobiw@suprafluid.com
 *     www: http://sourceforge.net/projects/avra
 *
 */


#include <stdlib.h>
#include <string.h>

#include "misc.h"
#include "avra.h"
#include "device.h"

#define DEV_VAR    "__DEVICE__"	// Device var name
#define FLASH_VAR  "__FLASH_SIZE__"	// Flash size var name
#define EEPROM_VAR "__EEPROM_SIZE__"	// EEPROM size var name
#define RAM_VAR    "__RAM_SIZE__"	// RAM size var name
#define DEV_PREFIX "__"		// Device name prefix
#define DEV_SUFFIX "__"		// Device name suffix
#define DEF_DEV_NAME "DEFAULT"	// Default device name (without prefix/suffix)
#define MAX_DEV_NAME 32		// Max device name length


struct device device_list[] =
{
  /*        Name, Flash(words),RAM start, RAM size, EEPROM, flags */
  {         NULL,     4194304,      0x60,  8388608,  65536, 0}, // Total instructions: 137
  /* ATtiny Series */
  // ATtiny4
  // ATtiny5
  // ATtiny9
  {   "ATtiny10",         512,      0x00,        0,      0, DF_NO_MUL|DF_NO_JMP|DF_TINY1X|DF_NO_XREG|DF_NO_YREG|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny11",         512,      0x00,        0,      0, DF_NO_MUL|DF_NO_JMP|DF_TINY1X|DF_NO_XREG|DF_NO_YREG|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny12",         512,      0x00,        0,     64, DF_NO_MUL|DF_NO_JMP|DF_TINY1X|DF_NO_XREG|DF_NO_YREG|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny13",         512,      0x60,       64,     64, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny13A",        512,      0x60,       64,     64, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny15",         512,      0x00,        0,     64, DF_NO_MUL|DF_NO_JMP|DF_TINY1X|DF_NO_XREG|DF_NO_YREG|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  // ATtiny20
  {   "ATtiny22",        1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny24",        1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny24A",       1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny25",        1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny26",        1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny28",        1024,      0x00,        0,      0, DF_NO_MUL|DF_NO_JMP|DF_TINY1X|DF_NO_XREG|DF_NO_YREG|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  // ATtiny43U
  {   "ATtiny44",        2048,      0x60,      256,    256, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny44A",       2048,      0x60,      256,    256, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny45",        2048,      0x60,      256,    256, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny84",        4096,      0x60,      512,    512, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  {   "ATtiny85",        4096,      0x60,      512,    512, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  // ATtiny87
  // ATtiny167
  // ATtiny261A
  // ATtiny461A
  // ATtiny861A
  { "ATtiny2313",        1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  { "ATtiny2313A",       1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  { "ATtiny4313",        2048,      0x60,      256,    256, DF_NO_MUL|DF_NO_JMP|DF_NO_ELPM|DF_NO_ESPM|DF_NO_EICALL|DF_NO_EIJMP},
  /* AT90 series */
  {  "AT90S1200",         512,      0x00,        0,     64, DF_NO_MUL|DF_NO_JMP|DF_TINY1X|DF_NO_XREG|DF_NO_YREG|DF_NO_LPM|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP}, // 137 - MUL(6) - JMP(2) - TINY(10)
  {  "AT90S2313",        1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {  "AT90S2323",        1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {  "AT90S2333",        1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {  "AT90S2343",        1024,      0x60,      128,    128, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {  "AT90S4414",        2048,      0x60,      256,    256, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {  "AT90S4433",        2048,      0x60,      128,    256, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {  "AT90S4434",        2048,      0x60,      256,    256, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {  "AT90S8515",        4096,      0x60,      512,    512, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP}, // 137 - MUL(6) - JMP(2) - LPM_X(2) - ELPM(3) - SPM - ESPM - MOVW - BREAK - EICALL - EIJMP = 118
  {  "AT90C8534",        4096,      0x60,      256,    512, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  {  "AT90S8535",        4096,      0x60,      512,    512, DF_NO_MUL|DF_NO_JMP|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_MOVW|DF_NO_BREAK|DF_NO_EICALL|DF_NO_EIJMP},
  /* AT90USB series*/
  // AT90USB168
  // AT90USB1287
  /* ATmega series */
  {    "ATmega8",        4096,      0x60,     1024,    512, DF_NO_JMP|DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  {  "ATmega161",        8192,      0x60,     1024,    512, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  {  "ATmega162",        8192,     0x100,     1024,    512, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  {  "ATmega163",        8192,      0x60,     1024,    512, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  {   "ATmega16",        8192,      0x60,     1024,    512, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  {  "ATmega323",       16384,      0x60,     2048,   1024, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM}, // 137 - EICALL - EIJMP - ELPM(3) - ESPM = 131 (Data sheet says 130 but it's wrong)
  { "ATmega328P",       16384,     0x100,     2048,   1024, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  {   "ATmega32",       16384,      0x60,     2048,   1024, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  {  "ATmega603",       32768,      0x60,     4096,   2048, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_MUL|DF_NO_MOVW|DF_NO_LPM_X|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_BREAK},
  {  "ATmega103",       65536,      0x60,     4096,   4096, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_MUL|DF_NO_MOVW|DF_NO_LPM_X|DF_NO_ELPM_X|DF_NO_SPM|DF_NO_ESPM|DF_NO_BREAK}, // 137 - EICALL - EIJMP - MUL(6) - MOVW - LPM_X(2) - ELPM_X(2) - SPM - ESPM - BREAK = 121
  {  "ATmega104",       65536,      0x60,     4096,   4096, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ESPM}, // Old name for mega128
  {  "ATmega128",       65536,     0x100,     4096,   4096, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ESPM}, // 137 - EICALL - EIJMP - ESPM = 134 (Data sheet says 133 but it's wrong)
  {   "ATmega48",        2048,     0x100,      512,    256, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  {   "ATmega88",        4096,     0x100,     1024,    512, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  {  "ATmega168",        8192,     0x100,     1024,    512, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  { "ATmega8515",        8192,      0x60,      512,    512, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_ESPM},
  /* Other */
  {      "AT94K",        8192,      0x60,    16384,      0, DF_NO_EICALL|DF_NO_EIJMP|DF_NO_ELPM|DF_NO_SPM|DF_NO_ESPM|DF_NO_BREAK}, // 137 - EICALL - EIJMP - ELPM(3) - SPM - ESPM - BREAK = 129
  {NULL, 0, 0, 0, 0}
};

static int LastDevice=0;

/*********************************************/
/* Define vars for device in LastDevice   */
/*********************************************/

static void def_dev(struct prog_info *pi)
{
   def_var(pi,DEV_VAR,LastDevice);
   def_var(pi,FLASH_VAR,device_list[LastDevice].flash_size);
   def_var(pi,EEPROM_VAR,device_list[LastDevice].eeprom_size);
   def_var(pi,RAM_VAR,device_list[LastDevice].ram_size);
}

struct device *get_device(struct prog_info *pi, char *name)
{
	int i = 1;

    LastDevice = 0;
	if(name == NULL) {
        def_dev(pi);
	    return(&device_list[0]);
	}
	while(device_list[i].name) {
		if(!nocase_strcmp(name, device_list[i].name)) {
			LastDevice=i;
            def_dev(pi);
			return(&device_list[i]);
		}
		i++;
	}
	def_dev(pi);
	return(NULL);
}

// Pre-define devices. B.A. : Return value change from void to int 
int predef_dev(struct prog_info *pi) 
{
	int i;
	char temp[MAX_DEV_NAME+1];
	def_dev(pi);
	for (i=0;(!i)||(device_list[i].name);i++) {
		strncpy(temp,DEV_PREFIX,MAX_DEV_NAME);
		if (!i) strncat(temp,DEF_DEV_NAME,MAX_DEV_NAME);
		else strncat(temp,device_list[i].name,MAX_DEV_NAME);
		strncat(temp,DEV_SUFFIX,MAX_DEV_NAME);
		/* B.A. : New. Forward references allowed. But check, if everything is ok ... */
		if(pi->pass==PASS_1) { /* Pass 1 */
      if(test_constant(pi,temp,NULL)!=NULL) {
        fprintf(stderr,"Error: Can't define symbol %s twice. Please don't use predefined symbols !\n", temp);
        return(False);
      }
      if(def_const(pi, temp, i)==False)
        return(False);
	  } else { /* Pass 2 */
			int j;
			if(get_constant(pi, temp, &j)==False) {   /* Defined in Pass 1 and now missing ? */
   	    fprintf(stderr,"Constant %s is missing in pass 2\n",temp);
 	      return(False);
			}
		  if(i != j) {
 	      fprintf(stderr,"Constant %s changed value from %d in pass1 to %d in pass 2\n",temp,j,i);
 	      return(False);
			}
			/* OK. definition is unchanged */
	  }
	}
	return(True);
}

void list_devices()
{
  int i = 1;
  printf("Device name | Flash size | RAM start | RAM size | EEPROM size |  Supported\n"
         "            |  (words)   | (bytes)   | (bytes)  |   (bytes)   | instructions\n"
         "------------+------------+-----------+----------+-------------+--------------\n"
         " (default)  |    %7d |    0x%04x |  %7d |       %5d |          %3d\n",
         device_list[0].flash_size,
         device_list[0].ram_start,
         device_list[0].ram_size,
         device_list[0].eeprom_size,
         count_supported_instructions(device_list[0].flag));
  while(device_list[i].name) {
    printf(" %-10s |    %7d |    0x%04x |  %7d |       %5d |          %3d\n",
           device_list[i].name,
           device_list[i].flash_size,
           device_list[i].ram_start,
           device_list[i].ram_size,
           device_list[i].eeprom_size,
           count_supported_instructions(device_list[i].flag));
    i++;
  }
}

/* end of device.c */

