/***********************************************************************
 *
 *  avra - Assembler for the Atmel AVR microcontroller series
 *
 *  Copyright (C) 1998-2006 Jon Anders Haugum, Tobias Weber
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
#include <stdarg.h>
#include <string.h>

#include "misc.h"
#include "args.h"
#include "avra.h"
#include "device.h"

#define debug 0

const char *title =
  "AVRA: advanced AVR macro assembler Version %i.%i.%i Build %i (%s)\n"
  "Copyright (C) 1998-2010. Check out README file for more info\n"
  "\n"
  "   AVRA is an open source assembler for Atmel AVR microcontroller family\n"
  "   It can be used as a replacement of 'AVRASM32.EXE' the original assembler\n"
  "   shipped with AVR Studio. We do not guarantee full compatibility for avra.\n"
  "\n"
  "   AVRA comes with NO WARRANTY, to the extent permitted by law.\n"
  "   You may redistribute copies of avra under the terms\n"
  "   of the GNU General Public License.\n"
  "   For more information about these matters, see the files named COPYING.\n"
  "\n";

const char *usage =
	"usage: avra [-f][O|M|I|G] output file type\n"
	"            [-o <filename>] output file name\n"
	"            [-l <filename>] generate list file\n"
	"            [-m <mapfile>] generate map file\n"
  "[--define <symbol>[=<value>]]  [--includedir <dir>] [--listmac]\n"
	"            [--max_errors <number>] [--devices] [--version]\n"
	"            [-h] [--help] general help\n"
	"            "
	"            <file to assemble>\n"
	"\n"
	"   --listfile    -l : Create list file\n"
	"   --mapfile     -m : Create map file\n"
	"   --define      -D : Define symbol.\n"
	"   --includepath  -I : Additional include paths.\n"
	"   --listmac        : List macro expansion in listfile.\n"
	"   --max_errors     : Maximum number of errors before exit\n"
  "                      (default: 10)\n"
	"   --devices        : List out supported devices.\n"
	"   --version        : Version information.\n"
	"   --help, -h       : This help text.\n"
	"\n"
  "Just replace the AVRASM32.EXE with AVRA.EXE in your\n"
  "AVRStudio directories to avra's binary.\n";

int main(int argc, char *argv[])
{
  int show_usage = False;
  struct prog_info *pi=NULL;
  struct args *args;
  unsigned char c;

#if debug == 1
  int i;
  for(i = 0; i < argc; i++) {
    printf(argv[i]);
    printf("\n");
  }
#endif

  printf(title, VER_MAJOR, VER_MINOR, VER_RELEASE, VER_BUILD, VER_DATE);

  args = alloc_args(ARG_COUNT);
  if(args) {
    define_arg(args, ARG_DEFINE,      ARGTYPE_STRING_MULTISINGLE,  'D', "define",      NULL);
    define_arg(args, ARG_INCLUDEPATH, ARGTYPE_STRING_MULTISINGLE,  'I', "includepath", NULL);
    define_arg(args, ARG_LISTMAC,     ARGTYPE_BOOLEAN,              0,  "listmac",     "1");
    define_arg(args, ARG_MAX_ERRORS,  ARGTYPE_STRING,               0,  "max_errors",  "10");
    define_arg(args, ARG_COFF,        ARGTYPE_BOOLEAN,              0,  "coff",        NULL);
    define_arg(args, ARG_DEVICES,     ARGTYPE_BOOLEAN,              0,  "devices",     NULL);
    define_arg(args, ARG_VER,         ARGTYPE_BOOLEAN,              0,  "version",     NULL);
    define_arg(args, ARG_HELP,        ARGTYPE_BOOLEAN,             'h', "help",        NULL);
    define_arg(args, ARG_WRAP,        ARGTYPE_BOOLEAN,             'w', "wrap",        NULL);	// Not implemented ? B.A.
    define_arg(args, ARG_WARNINGS,    ARGTYPE_STRING_MULTISINGLE,  'W', "warn",        NULL);
    define_arg(args, ARG_FILEFORMAT,  ARGTYPE_CHAR_ATTACHED,       'f', "filetype",    "0");	// Not implemented ? B.A.
    define_arg(args, ARG_LISTFILE,    ARGTYPE_STRING,              'l', "listfile",    NULL);
    define_arg(args, ARG_OUTFILE,     ARGTYPE_STRING,              'o', "outfile",     NULL);	// Not implemented ? B.A.
    define_arg(args, ARG_MAPFILE,     ARGTYPE_STRING,              'm', "mapfile",     NULL);
    define_arg(args, ARG_DEBUGFILE,   ARGTYPE_STRING,              'd', "debugfile",   NULL);	// Not implemented ? B.A.
    define_arg(args, ARG_EEPFILE,     ARGTYPE_STRING,              'e', "eepfile",     NULL);	// Not implemented ? B.A.


    c = read_args(args, argc, argv);
    
    if(c != 0) {
	  if(!GET_ARG(args, ARG_HELP) && (argc != 1))	{
	    if(!GET_ARG(args, ARG_VER)) {
		  if(!GET_ARG(args, ARG_DEVICES)) {
            pi = get_pi(args);
		    if(pi) {
              get_rootpath(pi, args);  /* get assembly root path */
			  if (assemble(pi) != 0) { /* the main assembly call */
				  exit(EXIT_FAILURE);
			  }
			  free_pi(pi);             /* free all allocated memory */
			}
		  }
		  else {
		    list_devices();            /* list all supported devices */
		  }
		}
	  }
	  else
	    show_usage = True;
	}
	free_args(args);
  }
  else {
	show_usage = True;
	printf("\n");
  }
  if(show_usage) {
	printf("%s", usage);
  }
  exit(EXIT_SUCCESS);
  return (0);  /* compiler warning, JEG 4-23-03 */
}

void get_rootpath(struct prog_info *pi, struct args *args)
{
  int i;
  int j;
  char c;
  struct data_list *data;
  
  data = args->first_data;
  if(!data)
	  return;
  while(data->next) data = ((data)->next);
  
  if (data != NULL) {
    i = strlen((char *)data->data);
    if (i > 0) {
      pi->root_path = malloc(i + 1);
      strcpy(pi->root_path,(char *)data->data);
      j = 0;
      do {
       c = pi->root_path[i];
       if(c == '\\' || c == '/') {
         j = i + 1;
         break;
       }
      } while(i-- > 0);
      pi->root_path[j] = '\0';
      return;
    }
  }
  pi->root_path = "";
}


int assemble(struct prog_info *pi) {
  unsigned char c;

  if(pi->args->first_data) {
	printf("Pass 1...\n");
	if(load_arg_defines(pi)==False)
		return -1;
	if(predef_dev(pi)==False) /* B.A.: Now with error check */
		return -1;
	/*** FIRST PASS ***/
	def_orglist(pi);			/* B.A. : Store first active segment and seg_addr (Default : Code, Adr=0) */
	c = parse_file(pi, (char *)pi->args->first_data->data);
	fix_orglist(pi);			/* B.A. : Update last active segment */
	test_orglist(pi);	 		/* B.A.: Test for overlapping memory segments and out of chip space */
	if(c != False) {
#if debug == 1
		printf("error_count = %i\n", pi->error_count);
#endif
		/* B.A.: This part is obsolete. Now check is done in test_orglist() */
		/* before we go to the 2nd pass, make sure used space is ok */
		/* if(pi->eseg_count > pi->device->eeprom_size) {
			print_msg(pi, MSGTYPE_ERROR, 
			"EEPROM space exceeded by %i bytes!", pi->eseg_count-pi->device->eeprom_size);
			return -1;
		}  
		if(pi->cseg_count > pi->device->flash_size) {
			print_msg(pi, MSGTYPE_ERROR, 
			"FLASH space exceeded by %i bytes!", pi->cseg_count-pi->device->flash_size);
			return -1;
		} */

		/* if there are no furter errors, we can continue with 2nd pass */
		if(pi->error_count == 0) {
			prepare_second_pass(pi);
			if(load_arg_defines(pi)==False)
				return -1;
			if(predef_dev(pi)==False)	/* B.A.: Now with error check */
				return -1;
			c = open_out_files(pi, pi->args->first_data->data);
			if(c != 0) {
				printf("Pass 2...\n");
				parse_file(pi, (char *)pi->args->first_data->data);
				printf("done\n\n");
				print_orglist(pi); 	/* B.A.: List used memory segments */
				if(GET_ARG(pi->args, ARG_COFF) && (pi->error_count == 0)) {
					write_coff_file(pi);
				}
				write_map_file(pi);
				if(pi->error_count) { /* if there were errors */
					printf("\nAssembly aborted with %d errors and %d warnings.\n", pi->error_count, pi->warning_count);
						unlink_out_files(pi, pi->args->first_data->data);
				} else { /* assembly was succesfull */
					if(pi->warning_count)
						printf("\nAssembly complete with no errors (%d warnings).\n", pi->warning_count);
					else
						printf("\nAssembly complete with no errors.\n");
					close_out_files(pi);
				}  
			}
		} else	{
			unlink_out_files(pi, pi->args->first_data->data);
		}  
	}
  } else {
	printf("Error: You need to specify a file to assemble\n");
  }
  return pi->error_count;
}


int load_arg_defines(struct prog_info *pi)
{
  int i;
  char *expr;
  char buff[256];
  struct data_list *define;

  for(define = GET_ARG(pi->args, ARG_DEFINE); define; define = define->next) {
	  strcpy(buff, define->data);
	  expr = get_next_token( buff, TERM_EQUAL);
	  if(expr) {
  	  // we reach this, when there is actually a value passed..	
	    if(!get_expr(pi, expr, &i)) {
	  	  return(False);
	    }
    } else { 
  	  // if user didnt specify a value, we default to 1 
  	  i = 1; 
    }
	/* B.A. : New. Forward references allowed. But check, if everything is ok ... */
    if(pi->pass==PASS_1) { /* Pass 1 */
      if(test_constant(pi,buff,NULL)!=NULL) {
        	fprintf(stderr,"Error: Can't define symbol %s twice\n", buff);
        	return(False);
      }
      if(def_const(pi, buff, i)==False)
        return(False);
		} else { /* Pass 2 */
			int j;
			if(get_constant(pi, buff, &j)==False) {   /* Defined in Pass 1 and now missing ? */
				fprintf(stderr,"Constant %s is missing in pass 2\n",buff);
				return(False);
			}
			if(i != j) {
				fprintf(stderr,"Constant %s changed value from %d in pass1 to %d in pass 2\n",buff,j,i);
				return(False);
			}
				/* OK. Definition is unchanged */
		}
	}
  return(True);
}

/******************************************
 * prog_info
 ******************************************/
struct prog_info *get_pi(struct args *args) {
	struct prog_info *pi;
	struct data_list *warnings;

	pi = (struct prog_info *)calloc(1, sizeof(struct prog_info));
	if(!pi) 
		return(NULL);
	memset(pi, 0, sizeof(struct prog_info));
	pi->args = args;
	pi->device = get_device(pi,NULL);
	if(GET_ARG(args, ARG_LISTFILE) == NULL) {
		pi->list_on = False;
	} else {
		pi->list_on = True;
	}
	if(GET_ARG(args, ARG_MAPFILE) == NULL) {
		pi->map_on = False;
	} else {
		pi->map_on = True;
	}
	for(warnings = GET_ARG(args, ARG_WARNINGS); warnings; warnings = warnings->next) {
		if(!nocase_strcmp(warnings->data, "NoRegDef"))
			pi->NoRegDef = 1;
	}
	pi->segment = SEGMENT_CODE;
	pi->dseg_addr = pi->device->ram_start;
	pi->max_errors = atoi(GET_ARG(args, ARG_MAX_ERRORS));
	pi->pass=PASS_1; 		/* B.A. : The pass variable is now stored in the pi struct */
	pi->time=time(NULL); 		/* B.A. : Now use a global timestamp  */
	return(pi);
}

void free_pi(struct prog_info *pi) {
  free_defs(pi);			/* B.A. : Now free in pi included structures first */
  free_labels(pi);
  free_constants(pi); 
  free_variables(pi);
  free_blacklist(pi);
  free_orglist(pi);
  free(pi);
}

void prepare_second_pass(struct prog_info *pi) {
  
  pi->segment = SEGMENT_CODE;
  pi->cseg_addr = 0;
  pi->dseg_addr = pi->device->ram_start;
  pi->eseg_addr = 0;
  //pi->macro_nstlblnr = 0;
  pi->pass=PASS_2; 			/* B.A. : Change to pass 2. Now stored in pi struct. */
  free_defs(pi);
  // free_constants(pi);		/* B.A. : Now don't kill stored constants. We need them in the second pass now */
  free_variables(pi);
}


void print_msg(struct prog_info *pi, int type, char *fmt, ... )
{
	char *pc;
	if(type == MSGTYPE_OUT_OF_MEM) {
		fprintf(stderr, "Error: Unable to allocate memory!\n");
	} else {
		if(type != MSGTYPE_APPEND) { 				/* B.A. Added for .message directive */
			if((pi->fi != NULL) && (pi->fi->include_file->name != NULL)) { 	/* B.A.: Skip, if filename or fi is NULL (Bug 1462900) */
				/* check if adding path name is needed*/
				pc = strstr(pi->fi->include_file->name, pi->root_path);
				if(pc == NULL) { 
					fprintf(stderr, "%s%s(%d) : ", pi->root_path ,pi->fi->include_file->name, pi->fi->line_number);
				} else {
					fprintf(stderr, "%s(%d) : ", pi->fi->include_file->name, pi->fi->line_number);
				}
			}
		}
		switch(type) {
			case MSGTYPE_ERROR:
				pi->error_count++;
				fprintf(stderr, "Error   : ");
				break;
			case MSGTYPE_WARNING:
				pi->warning_count++;
				fprintf(stderr, "Warning : ");
				break;
			case MSGTYPE_MESSAGE:
/*			case MSGTYPE_MESSAGE_NO_LF:
			case MSGTYPE_APPEND: */
				break;
		}
		if(type != MSGTYPE_APPEND) { /* B.A. Added for .message directive */
			if(pi->macro_call) {
				fprintf(stderr, "[Macro: %s: %d:] ", pi->macro_call->macro->include_file->name,
					pi->macro_call->line_index + pi->macro_call->macro->first_line_number);
			}
		}
		if(fmt != NULL) {
			va_list args;
			va_start(args, fmt);			vfprintf(stderr, fmt, args);			va_end(args);
		}

		if( (type != MSGTYPE_APPEND) && (type != MSGTYPE_MESSAGE_NO_LF) )  /* B.A. Added for .message directive */
			fprintf(stderr, "\n");
	}
}


/* B.A. : New functions to create / search / remove constant, variables, labels */
/* def_const, def_var moved from device.c to this place */
int def_const(struct prog_info *pi, const char *name, int value) 
{
	struct label *label;
	label = malloc(sizeof(struct label));
	if(!label) {
		print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
		return(False);
	}
	label->next = NULL;
	if(pi->last_constant)
		pi->last_constant->next = label;
	else
		pi->first_constant = label;
	pi->last_constant = label;
	label->name = malloc(strlen(name) + 1);
	if(!label->name) {
		print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
		return(False);
	}
	strcpy(label->name, name);
	label->value = value;
	return(True);
}

int def_var(struct prog_info *pi, char *name, int value) 
{
	struct label *label;

 	for(label = pi->first_variable; label; label = label->next)
		if(!nocase_strcmp(label->name, name)) {
			label->value = value;
			return(True);
		}
	label = malloc(sizeof(struct label));
	if(!label) {
		print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
		return(False);
	}
	label->next = NULL;
	if(pi->last_variable)
		pi->last_variable->next = label;
	else
		pi->first_variable = label;
	pi->last_variable = label;
	label->name = malloc(strlen(name) + 1);
	if(!label->name) {
		print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
		return(False);
	}
	strcpy(label->name, name);
	label->value = value;
	return(True);
}


int def_blacklist(struct prog_info *pi, const char *name) 
{
	struct label *label;
	label = malloc(sizeof(struct label));
	if(!label) {
		print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
		return(False);
	}
	label->next = NULL;
	if(pi->last_blacklist)
		pi->last_blacklist->next = label;
	else
		pi->first_blacklist = label;
	pi->last_blacklist = label;
	label->name = malloc(strlen(name) + 1);
	if(!label->name) {
		print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
		return(False);
	}
	strcpy(label->name, name);
	label->value = 0;
	return(True);
}

/* B.A.: Store programmed areas for later check */
int def_orglist(struct prog_info *pi) 
{
	struct orglist *orglist;
	if(pi->pass != PASS_1)
		return(True);
	orglist = malloc(sizeof(struct orglist));
	if(!orglist) {
		print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
		return(False);
	}
	orglist->next = NULL;
	if(pi->last_orglist)
		pi->last_orglist->next = orglist;
	else
		pi->first_orglist = orglist;
	pi->last_orglist = orglist;
	orglist->segment=pi->segment;
	switch(pi->segment) {
		case SEGMENT_CODE:
			orglist->start = pi->cseg_addr;
			break;
		case SEGMENT_DATA:
			orglist->start = pi->dseg_addr;
			break;
		case SEGMENT_EEPROM:
			orglist->start = pi->eseg_addr;
	}
	orglist->length=0;
	return(True);
}

/* B.A.: Fill length entry of last orglist */
int fix_orglist(struct prog_info *pi) 
{
	if(pi->pass != PASS_1)
		return(True);
	if((pi->last_orglist == NULL) || (pi->last_orglist->length!=0)) {
        	fprintf(stderr,"Internal Error: fix_orglist\n");
		return(False);
	}
	pi->last_orglist->segment=pi->segment;
	switch(pi->segment) {
		case SEGMENT_CODE:
			pi->last_orglist->length = pi->cseg_addr - pi->last_orglist->start;
			break;
		case SEGMENT_DATA:
			pi->last_orglist->length = pi->dseg_addr - pi->last_orglist->start;
			break;
		case SEGMENT_EEPROM:
			pi->last_orglist->length = pi->eseg_addr - pi->last_orglist->start;
	}
	return(True);
}

/* B.A.: Debug output of orglist */
void print_orglist(struct prog_info *pi)
{
	struct orglist *orglist=pi->first_orglist;
	printf("Used memory blocks:\n");
	while(orglist!=NULL) {
		if(orglist->length) { /* Skip blocks with size == 0 */
			switch(orglist->segment) {
				case SEGMENT_CODE:
					printf("   Code  "); break;
				case SEGMENT_DATA:
					printf("   Data  "); break;
				case SEGMENT_EEPROM:
					printf("   EEPROM"); break;
				printf("INVALID SEGMENT DATA !\n");
			}	
			printf("    :  Start = 0x%04X, End = 0x%04X, Length = 0x%04X\n",
				orglist->start,orglist->start+orglist->length-1,orglist->length);
		}
		orglist=orglist->next;
	}
}

/* B.A.: Test for overlapping segments and device space */
int test_orglist(struct prog_info *pi)
{
	struct orglist *orglist2,*orglist=pi->first_orglist;
	int error_count=0;
	if(pi->device->name==NULL) {
		fprintf(stderr,"Warning : No .DEVICE definition found. Cannot make useful address range check !\n");
		pi->warning_count++;
	}
	while(orglist!=NULL) {
		if(orglist->length) { /* Skip blocks with size == 0 */
			// printf("Segment %d,  Start = %5d, Length = %5d\n",orglist->segment,orglist->start,orglist->length);
			/* Make sure address area is valid */
			switch(orglist->segment) {
				case SEGMENT_CODE:
					if((orglist->start + orglist->length) > pi->device->flash_size) {						
						fprintf(stderr,"Code segment exceeds valid address range [0..0x%04X] :",
							pi->device->flash_size-1);
						fprintf(stderr,"  Start = 0x%04X, End = 0x%04X, Length = 0x%04X\n",
							orglist->start,orglist->start+orglist->length-1,orglist->length);
						error_count++;
					}
					break;
				case SEGMENT_DATA:
					if(pi->device->ram_size == 0) {
						// Error message is generated in .DSEG directive. Skip ...
						// fprintf(stderr,"This device has no RAM. Don't use .DSEG \n");
						// error_count++;
						break;
					} 	// Fix bug 1742436. Added missing pi->device->ram_start
					if(((orglist->start + orglist->length) > (pi->device->ram_size + pi->device->ram_start)) ||
					    (orglist->start < pi->device->ram_start)) {
						fprintf(stderr,"Data segment exceeds valid address range [0x%04X..0x%04X] :",
							pi->device->ram_start,pi->device->ram_start+pi->device->ram_size-1);
						fprintf(stderr,"  Start = 0x%04X, End = 0x%04X, Length = 0x%04X\n",
							orglist->start,orglist->start+orglist->length-1,orglist->length);
						error_count++;
					}
					break;
				case SEGMENT_EEPROM: // Fix bug 1742437 : replace ram_size by eeprom_size
					if(pi->device->eeprom_size == 0) {
						// Error message is generated in .ESEG directive. Skip ...
						// fprintf(stderr,"This device has no EEPROM. Don't use .ESEG !\n");
						// error_count++;
						break;
					}
					if((orglist->start + orglist->length) > pi->device->eeprom_size) {
						fprintf(stderr,"EEPROM segment exceeds valid address range [0..0x%04X] :",
							pi->device->eeprom_size-1);
						fprintf(stderr,"  Start = 0x%04X, End = 0x%04X, Length = 0x%04X\n",
							orglist->start,orglist->start+orglist->length-1,orglist->length);
						error_count++;
					}
					break;
			}
			/* Overlap-test */
			orglist2=orglist->next;
			while(orglist2!=NULL) {
				if((orglist != orglist2) && (orglist2->length) && (orglist->segment == orglist2->segment)) {
					// printf("<> Segment %d,  Start = %5d, Length = %5d\n",orglist2->segment,orglist2->start,orglist2->length);
					if((orglist->start  < (orglist2->start + orglist2->length)) &&
					   (orglist2->start < ( orglist->start +  orglist->length))) {
						fprintf(stderr,"Error: Overlapping ");
						switch(orglist->segment) {
							case SEGMENT_CODE:
								fprintf(stderr,"Code"); break;
							case SEGMENT_DATA:
								fprintf(stderr,"Data"); break;
							case SEGMENT_EEPROM:
								fprintf(stderr,"EEPROM"); break;
						}
						fprintf(stderr,"-segments :\n");
						fprintf(stderr,"  Start = 0x%04X, End = 0x%04X, Length = 0x%04X\n",
							orglist->start,orglist->start+orglist->length-1,orglist->length);
						fprintf(stderr,"  Start = 0x%04X, End = 0x%04X, Length = 0x%04X\n",
							orglist2->start,orglist2->start+orglist2->length-1,orglist2->length);
						fprintf(stderr,"Please check your .ORG directives !\n");
						error_count++;
					}
				}				
				orglist2=orglist2->next;
			}
		}
		orglist=orglist->next;
	}
	if(!error_count)
		return(True);
	pi->error_count+=error_count;
	return(False);
}



/* Get the value of a label. Return FALSE if label was not found */
int get_label(struct prog_info *pi,char *name,int *value)
{
  struct label *label=search_symbol(pi,pi->first_label,name,NULL);
	if(label==NULL) return False;
	if(value!=NULL)	*value=label->value;	
	return True;
}

int get_constant(struct prog_info *pi,char *name,int *value)
{
  struct label *label=search_symbol(pi,pi->first_constant,name,NULL);
	if(label==NULL) return False;
	if(value!=NULL)	*value=label->value;	
	return True;
}

int get_variable(struct prog_info *pi,char *name,int *value)
{
  struct label *label=search_symbol(pi,pi->first_variable,name,NULL);
	if(label==NULL) return False;
	if(value!=NULL)	*value=label->value;	
	return True;
}

/* Test, if label exists. Return NULL -> not defined, else return the pointer to label struct */
/* If message != NULL print error message if symbol is defined */
struct label *test_label(struct prog_info *pi,char *name,char *message)
{
	return search_symbol(pi,pi->first_label,name,message);
}

struct label *test_constant(struct prog_info *pi,char *name,char *message)
{
	return search_symbol(pi,pi->first_constant,name,message);
}

struct label *test_variable(struct prog_info *pi,char *name,char *message)
{
	return search_symbol(pi,pi->first_variable,name,message);
}

struct label *test_blacklist(struct prog_info *pi,char *name,char *message)
{
	return search_symbol(pi,pi->first_blacklist,name,message);
}

/* Search in label,constant,variable,blacklist - list for a matching entry */
/* Use first = pi->first_label,first_constant,first_variable,first_blacklist to select list */
/* If message != NULL Print error message if symbol is defined */
struct label *search_symbol(struct prog_info *pi,struct label *first,char *name,char *message)
{
	struct label *label;
	for(label = first; label; label = label->next)
		if(!nocase_strcmp(label->name, name)) {
			if(message) {
				print_msg(pi, MSGTYPE_ERROR, message, name);				   
			}
			return(label);
		}
	return(NULL);
}


void free_defs(struct prog_info *pi)
{
  struct def *def, *temp_def;
  for(def = pi->first_def; def;) {
	  temp_def = def;
	  def = def->next;
	  free(temp_def->name);
	  free(temp_def);
  }
  pi->first_def = NULL;	
  pi->last_def = NULL;	
}

void free_labels(struct prog_info *pi)
{
  struct label *label, *temp_label;
  for(label = pi->first_label; label;) {
    temp_label = label;
	  label = label->next;
	  free(temp_label->name);
	  free(temp_label);
  }
  pi->first_label = NULL;
  pi->last_label = NULL;
}

void free_constants(struct prog_info *pi)
{
  struct label *label, *temp_label;
  for(label = pi->first_constant; label;) {
    temp_label = label;
	  label = label->next;
	  free(temp_label->name);
	  free(temp_label);
  }
  pi->first_constant = NULL;
  pi->last_constant = NULL;
}

void free_blacklist(struct prog_info *pi)
{
  struct label *label, *temp_label;
  for(label = pi->first_blacklist; label;) {
	  temp_label = label;
	  label = label->next;
	  free(temp_label->name);
	  free(temp_label);
  }
  pi->first_blacklist = NULL;
  pi->last_blacklist = NULL;
}

void free_variables(struct prog_info *pi)
{
  struct label *label, *temp_label;
  for(label = pi->first_variable; label;) {
	  temp_label = label;
	  label = label->next;
	  free(temp_label->name);
	  free(temp_label);
  }
  pi->first_variable = NULL;
  pi->last_variable = NULL;
}

void free_orglist(struct prog_info *pi)
{
  struct orglist *orglist, *temp_orglist;
  for(orglist = pi->first_orglist; orglist;) {
	  temp_orglist = orglist;
	  orglist = orglist->next;
	  free(temp_orglist);
  }
  pi->first_orglist = NULL;
  pi->last_orglist = NULL;
}


/* avra.c */

