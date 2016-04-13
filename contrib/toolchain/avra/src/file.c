/***********************************************************************
 *
 *  avra - Assembler for the Atmel AVR microcontroller series
 *
 *  Copyright (C) 1998-2004 Jon Anders Haugum, Tobias Weber
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h> /* B.A. for unlink function */


#include "misc.h"
#include "avra.h"
#include "args.h"


int open_out_files(struct prog_info *pi, char *filename)
{
	int length;
	char *buff;
	int ok = True; /* flag for coff results */

	length = strlen(filename);
	buff = malloc(length + 9);
	if(buff == NULL) {
	  print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
    return(False);
  }
  strcpy(buff, filename);
  if(length < 4) {
    printf("Error: wrong input file name\n");
  }
  if(!nocase_strcmp(&buff[length - 4], ".asm")) {
	  length -= 4;
	  buff[length] = '\0';
	}
  //printf("pi->cseg_count = %i\n", pi->cseg_count);
  //printf("pi->eseg_count = %i\n", pi->eseg_count);
	
	/* open files for code output */
  strcpy(&buff[length], ".hex");
	if(!(pi->hfi = open_hex_file(buff))) { /* check if open failed */
    print_msg(pi, MSGTYPE_ERROR, "Could not create output hex file!");
    ok = False;
  }
  strcpy(&buff[length], ".obj");
  if(!(pi->obj_file = open_obj_file(pi, buff))) {
    print_msg(pi, MSGTYPE_ERROR, "Could not create object file!");
    ok = False;
  }  
	
	/* open files for eeprom output */
  strcpy(&buff[length], ".eep.hex");
  if(!(pi->eep_hfi = open_hex_file(buff))) {
		print_msg(pi, MSGTYPE_ERROR, "Could not create eeprom hex file!");
    ok = False;
  }  
  /* coff file is always generated */
  strcpy(&buff[length], ".cof");
	pi->coff_file = open_coff_file(pi, buff);
  /* open list file */
	if (pi->list_on) {
    strcpy(buff, GET_ARG(pi->args, ARG_LISTFILE));
    pi->list_file = fopen(buff, "w");
    if(pi->list_file == NULL) {
			print_msg(pi, MSGTYPE_ERROR, "Could not create list file!");
      ok = False;
    }
    /* write list file header */
    fprintf(pi->list_file, "\nAVRA   Ver. %i.%i.%i %s %s\n\n",VER_MAJOR, VER_MINOR, VER_RELEASE, filename, ctime(&pi->time));
  }
  else {
    pi->list_file = NULL;
  }
  free(buff);

	if(ok)
	  return(True);
  else
	  close_out_files(pi);
  return(False);
}

/* delete all output files */
void unlink_out_files(struct prog_info *pi, char *filename)
{
	char *buff;
	int length;

  close_out_files(pi);

	length = strlen(filename);
	buff = malloc(length + 9);
	if(buff == NULL) {
	  print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
    return;
  }
  strcpy(buff, filename);
  if(!nocase_strcmp(&buff[length - 4], ".asm")) {
	  length -= 4;
	  buff[length] = '\0';
	}
#if debug == 1
  printf("unlinking files");
#endif
  strcpy(&buff[length], ".hex");
  unlink(buff);
  strcpy(&buff[length], ".obj");
  unlink(buff);
  strcpy(&buff[length], ".eep.hex");
  unlink(buff);
  strcpy(&buff[length], ".cof");
  unlink(buff);
  strcpy(&buff[length], ".lst");
  unlink(buff);
  strcpy(&buff[length], ".map");
  unlink(buff);
}  

void close_out_files(struct prog_info *pi)
{
  char stmp[2048];
  
	if(pi->error_count == 0) {
		sprintf(stmp,
      "Segment usage:\n"
		  "   Code      :   %7d words (%d bytes)\n"
		  "   Data      :   %7d bytes\n"
		  "   EEPROM    :   %7d bytes\n",
		  pi->cseg_count, pi->cseg_count * 2, pi->dseg_count, pi->eseg_count);
    printf("%s", stmp);
	}
	if(pi->hfi)
    close_hex_file(pi->hfi);
	if(pi->eep_hfi)
    close_hex_file(pi->eep_hfi);
	if(pi->list_file) {
	  fprintf(pi->list_file, "\n\n%s", stmp);
		if(pi->error_count == 0)
			fprintf(pi->list_file, "\nAssembly completed with no errors.\n");
		fclose(pi->list_file);
	}
	if(pi->obj_file)
    close_obj_file(pi, pi->obj_file);
	if(pi->coff_file)
    close_coff_file(pi, pi->coff_file);
}


struct hex_file_info *open_hex_file(char *filename)
{
	struct hex_file_info *hfi;

	hfi = calloc(1, sizeof(struct hex_file_info));
	if(hfi) {
		hfi->segment = -1;
		hfi->fp = fopen(filename, "wb");
		if(!hfi->fp) {
			close_hex_file(hfi);
			hfi = NULL;
		}
	}
	return(hfi);
}


void close_hex_file(struct hex_file_info *hfi)
{
	if(hfi->fp) {
		if(hfi->count != 0)
		  do_hex_line(hfi);
		fprintf(hfi->fp, ":00000001FF\x0d\x0a");
		fclose(hfi->fp);
	}
	free(hfi);
}


void write_ee_byte(struct prog_info *pi, int address, unsigned char data)
{
	if((pi->eep_hfi->count == 16) || ((address != (pi->eep_hfi->linestart_addr + pi->eep_hfi->count)) && (pi->eep_hfi->count != 0)))
		do_hex_line(pi->eep_hfi);
	if(pi->eep_hfi->count == 0)
		pi->eep_hfi->linestart_addr = address;
	pi->eep_hfi->hex_line[pi->eep_hfi->count++] = data;

	if(pi->coff_file)
	        write_coff_eeprom(pi, address, data);
}

void write_prog_word(struct prog_info *pi, int address, int data)
{
	write_obj_record(pi, address, data);
	address *= 2;
	if(pi->hfi->segment != (address >> 16))	{
	  if(pi->hfi->count != 0)
	    do_hex_line(pi->hfi);
	  pi->hfi->segment = address >> 16;
	  if(pi->hfi->segment >= 16) // Use 04 record for addresses above 1 meg since 02 can support max 1 meg
	    fprintf(pi->hfi->fp, ":02000004%04X%02X\x0d\x0a", pi->hfi->segment & 0xffff,
	            (0 - 2 - 4 - ((pi->hfi->segment >> 8) & 0xff) - (pi->hfi->segment & 0xff)) & 0xff);
	  else // Use 02 record for addresses below 1 meg since more programmers know about the 02 instead of the 04
	    fprintf(pi->hfi->fp, ":02000002%04X%02X\x0d\x0a", (pi->hfi->segment << 12) & 0xffff,
	            (0 - 2 - 2 - ((pi->hfi->segment << 4) & 0xf0)) & 0xff);
	}
	if((pi->hfi->count == 16) || ((address != (pi->hfi->linestart_addr + pi->hfi->count)) && (pi->hfi->count != 0)))
		do_hex_line(pi->hfi);
	if(pi->hfi->count == 0)
		pi->hfi->linestart_addr = address;
	pi->hfi->hex_line[pi->hfi->count++] = data & 0xff;
	pi->hfi->hex_line[pi->hfi->count++] = (data >> 8) & 0xff;

	if(pi->coff_file != 0)
	        write_coff_program(pi, address, data);
}


void do_hex_line(struct hex_file_info *hfi)
{
	int i;
	unsigned char checksum = 0;

	fprintf(hfi->fp, ":%02X%04X00", hfi->count, hfi->linestart_addr & 0xffff);
	checksum -= hfi->count + ((hfi->linestart_addr >> 8) & 0xff) + (hfi->linestart_addr & 0xff);
	for(i = 0; i < hfi->count; i++)	{
		fprintf(hfi->fp, "%02X", hfi->hex_line[i]);
		checksum -= hfi->hex_line[i];
	}
	fprintf(hfi->fp, "%02X\x0d\x0a", checksum);
	hfi->count = 0;
}


FILE *open_obj_file(struct prog_info *pi, char *filename)
{
	int i;
	FILE *fp;
	struct include_file *include_file;

	fp = fopen(filename, "wb");
	if(fp) {
		i = pi->cseg_count * 9 + 26;
		fputc((i >> 24) & 0xff, fp);
		fputc((i >> 16) & 0xff, fp);
		fputc((i >> 8) & 0xff, fp);
		fputc(i & 0xff, fp);
		i = 26;
		fputc((i >> 24) & 0xff, fp);
		fputc((i >> 16) & 0xff, fp);
		fputc((i >> 8) & 0xff, fp);
		fputc(i & 0xff, fp);
		fputc(9, fp);
		i = 0;
		for(include_file = pi->first_include_file; include_file; include_file = include_file->next)
			i++;
		fputc(i, fp);
		fprintf(fp, "AVR Object File");
		fputc('\0', fp);
	}
	return(fp);
}


void close_obj_file(struct prog_info *pi, FILE *fp)
{
	struct include_file *include_file;

	for(include_file = pi->first_include_file; include_file; include_file = include_file->next) {
		fprintf(fp, "%s", include_file->name);
		fputc('\0', fp);
	}
	fputc('\0', fp);
	fclose(fp);
}


void write_obj_record(struct prog_info *pi, int address, int data)
{
	fputc((address >> 16) & 0xff, pi->obj_file);
	fputc((address >> 8) & 0xff, pi->obj_file);
	fputc(address & 0xff, pi->obj_file);
	fputc((data >> 8) & 0xff, pi->obj_file);
	fputc(data & 0xff, pi->obj_file);
	fputc(pi->fi->include_file->num & 0xff, pi->obj_file);
	fputc((pi->fi->line_number >> 8) & 0xff, pi->obj_file);
	fputc(pi->fi->line_number & 0xff, pi->obj_file);
	if(pi->macro_call)
		fputc(1, pi->obj_file);
	else
		fputc(0, pi->obj_file);
}

/* end of file.c */

