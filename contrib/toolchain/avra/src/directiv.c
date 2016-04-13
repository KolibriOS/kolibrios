/***********************************************************************
// Modified at line 252 to print out DW value in list file by davidrjburke@hotmail.com 11 Nov 2005
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
#include <ctype.h>

#include "misc.h"
#include "args.h"
#include "avra.h"
#include "device.h"

enum
{
	DIRECTIVE_BYTE = 0,
	DIRECTIVE_CSEG,
	DIRECTIVE_CSEGSIZE,
	DIRECTIVE_DB,
	DIRECTIVE_DEF,
	DIRECTIVE_DEVICE,
	DIRECTIVE_DSEG,
	DIRECTIVE_DW,
	DIRECTIVE_ENDM,
	DIRECTIVE_ENDMACRO,
	DIRECTIVE_EQU,
	DIRECTIVE_ESEG,
	DIRECTIVE_EXIT,
	DIRECTIVE_INCLUDE,
	DIRECTIVE_INCLUDEPATH,
	DIRECTIVE_LIST,
	DIRECTIVE_LISTMAC,
	DIRECTIVE_MACRO,
	DIRECTIVE_NOLIST,
	DIRECTIVE_ORG,
	DIRECTIVE_SET,
	DIRECTIVE_DEFINE,
	DIRECTIVE_UNDEF,
	DIRECTIVE_IFDEF,
	DIRECTIVE_IFNDEF,
	DIRECTIVE_IF,
	DIRECTIVE_ELSE,
	DIRECTIVE_ELSEIF,			/* B.A. : The Atmel AVR Assembler version 1.71 and later use ELSEIF and not ELIF */
	DIRECTIVE_ELIF,			
	DIRECTIVE_ENDIF,
	DIRECTIVE_MESSAGE,
	DIRECTIVE_WARNING,
	DIRECTIVE_ERROR,
	DIRECTIVE_PRAGMA,
	DIRECTIVE_COUNT
};

char *directive_list[] =
{
	"BYTE",
	"CSEG",
	"CSEGSIZE",
	"DB",
	"DEF",
	"DEVICE",
	"DSEG",
	"DW",
	"ENDM",
	"ENDMACRO",
	"EQU",
	"ESEG",
	"EXIT",
	"INCLUDE",
	"INCLUDEPATH",
	"LIST",
	"LISTMAC",
	"MACRO",
	"NOLIST",
	"ORG",
	"SET",
	"DEFINE",
	"UNDEF",
	"IFDEF",
	"IFNDEF",
	"IF",
	"ELSE",
	"ELSEIF",		/* B.A. : The Atmel AVR Assembler version 1.71 and later use ELSEIF and not ELIF */
	"ELIF",
	"ENDIF",
	"MESSAGE",
	"WARNING",
	"ERROR",
	"PRAGMA"
};


int parse_directive(struct prog_info *pi)
{
	int directive;
	int ok = True;
	int i;
	char *next, *data;
	struct file_info *fi_bak;

	struct def *def;
	struct data_list *incpath, *dl;

	next = get_next_token(pi->fi->scratch, TERM_SPACE);

	for(i = 0; pi->fi->scratch[i] != '\0'; i++) {
		pi->fi->scratch[i] = toupper(pi->fi->scratch[i]);
	}
	directive = get_directive_type(pi->fi->scratch + 1);
	if(directive == -1) {
		print_msg(pi, MSGTYPE_ERROR, "Unknown directive: %s", pi->fi->scratch);
		return(True);
	}
	switch(directive) {
		case DIRECTIVE_BYTE:
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, ".BYTE needs a size operand");
				return(True);
			}
			if(pi->segment != SEGMENT_DATA)
				print_msg(pi, MSGTYPE_ERROR, ".BYTE directive can only be used in data segment (.DSEG)");
			get_next_token(next, TERM_END);
			if(!get_expr(pi, next, &i))
				return(False);
			if((pi->pass == PASS_2) && pi->list_line && pi->list_on) {
				fprintf(pi->list_file, "D:%06x    %s\n", pi->dseg_addr, pi->list_line);
				pi->list_line = NULL;
			}
			pi->dseg_addr += i;
			if(pi->pass == PASS_1)
				pi->dseg_count += i;
			break;
		case DIRECTIVE_CSEG:
			fix_orglist(pi);
			pi->segment = SEGMENT_CODE;
			def_orglist(pi);
			break;
		case DIRECTIVE_CSEGSIZE:
			break;
		case DIRECTIVE_DB:
			if((pi->pass == PASS_2) && pi->list_line && pi->list_on) {
				fprintf(pi->list_file, "          %s\n", pi->list_line);
				pi->list_line = NULL;
			}
			return(parse_db(pi, next));
//			break;
		/* Directive .def */
		case DIRECTIVE_DEF:
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, ".DEF needs an operand");
				return(True);
			}
			data = get_next_token(next, TERM_EQUAL);
			if(!(data && (tolower(data[0]) == 'r') && isdigit(data[1]))) {
				print_msg(pi, MSGTYPE_ERROR, "%s needs a register (e.g. .def BZZZT = r16)", next);
				return(True);
			}
			i = atoi(&data[1]);
			/* check range of given register */
			if(i > 31)
				print_msg(pi, MSGTYPE_ERROR, "R%d is not a valid register", i);
			/* check if this reg is already assigned */
			for(def = pi->first_def; def; def = def->next) {
				if(def->reg == i && pi->pass == PASS_1 && !pi->NoRegDef) {
				    print_msg(pi, MSGTYPE_WARNING, "r%d is already assigned to '%s'!", i, def->name);
					return(True);
				}
			}
			/* check if this regname is already defined */
			for(def = pi->first_def; def; def = def->next) {
				if(!nocase_strcmp(def->name, next)) {
					if(pi->pass == PASS_1 && !pi->NoRegDef) {
						print_msg(pi, MSGTYPE_WARNING, "'%s' is already assigned as r%d but will now be set to r%i!", next, def->reg, i);
					}
					def->reg = i;
					return(True);
				}
			}
			/* B.A.: Check, if symbol is already defined as a label or constant */
			if(pi->pass == PASS_2) {
				if(get_label(pi,next,NULL))
					print_msg(pi, MSGTYPE_WARNING, "Name '%s' is used for a register and a label", next);
				if(get_constant(pi,next,NULL))
					print_msg(pi, MSGTYPE_WARNING, "Name '%s' is used for a register and a constant", next);
			}

			def = malloc(sizeof(struct def));
			if(!def) {
				print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
				return(False);
			}
			def->next = NULL;
			if(pi->last_def)
				pi->last_def->next = def;
			else
				pi->first_def = def;
			pi->last_def = def;
			def->name = malloc(strlen(next) + 1);
			if(!def->name) {
				print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
				return(False);
			}
			strcpy(def->name, next);
			def->reg = i;
			break;
		case DIRECTIVE_DEVICE:
			if(pi->pass == PASS_2)
				return(True);
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, ".DEVICE needs an operand");
				return(True);
			}
			if(pi->device->name!=NULL) { /* B.A.: Check for multiple device definitions */
				print_msg(pi, MSGTYPE_ERROR, "More than one .DEVICE definition");
			}
			if(pi->cseg_count || pi->dseg_count || pi->eseg_count) { /* B.A.: Check if something was already assembled */
				print_msg(pi, MSGTYPE_ERROR, ".DEVICE definition must be before any code lines");
			} else {
				if(pi->cseg_addr  || pi->eseg_addr || (pi->dseg_addr != pi->device->ram_start)) { /* B.A.: Check if something was already assembled */
					print_msg(pi, MSGTYPE_ERROR, ".DEVICE definition must be before any .ORG directive");
				}
			}

			get_next_token(next, TERM_END);
			pi->device = get_device(pi,next);
			if(!pi->device) {
				print_msg(pi, MSGTYPE_ERROR, "Unknown device: %s", next);
				pi->device = get_device(pi,NULL); /* B.A.: Fix segmentation fault if device is unknown */
			}

			/* Now that we know the device type, we can
			 * start memory allocation from the correct offsets.
			 */
			fix_orglist(pi);
			pi->cseg_addr = 0;
			pi->dseg_addr = pi->device->ram_start;
			pi->eseg_addr = 0;
			def_orglist(pi);
			break;
		case DIRECTIVE_DSEG:
			fix_orglist(pi);
			pi->segment = SEGMENT_DATA;
			def_orglist(pi);
			if(pi->device->ram_size == 0) {
				print_msg(pi, MSGTYPE_ERROR, "Can't use .DSEG directive because device has no RAM");
			}
			break;
		case DIRECTIVE_DW:
			if(pi->segment == SEGMENT_DATA) {
				print_msg(pi, MSGTYPE_ERROR, "Can't use .DW directive in data segment (.DSEG)");
				return(True);
			}
			while(next) {
				data = get_next_token(next, TERM_COMMA);
				if(pi->pass == PASS_2) {
				  if(!get_expr(pi, next, &i))
				    return(False);
				  if((i < -32768) || (i > 65535))
				    print_msg(pi, MSGTYPE_WARNING, "Value %d is out of range (-32768 <= k <= 65535). Will be masked", i);
                }
				if(pi->segment == SEGMENT_EEPROM) {
					if(pi->pass == PASS_2) {
						write_ee_byte(pi, pi->eseg_addr, (unsigned char)i);
						write_ee_byte(pi, pi->eseg_addr + 1, (unsigned char)(i >> 8));
					}
					pi->eseg_addr += 2;
					if(pi->pass == PASS_1)
						pi->eseg_count += 2;
				}
				// Modified by David Burke to print DW word in list file 4/Nov/2005
				else {
					if((pi->pass == PASS_2) && pi->hfi) {
						write_prog_word(pi, pi->cseg_addr, i);
						// Actual fiddling 
						if((pi->pass == PASS_2) && pi->list_line && pi->list_on) {
							fprintf(pi->list_file, "          %s\n", pi->list_line);
							pi->list_line = NULL;
							fprintf(pi->list_file, "C:%06x %04x\n", pi->cseg_addr,i);
						}
					}
					pi->cseg_addr++;
					if(pi->pass == PASS_1) pi->cseg_count++;
				}
				// End of Modification by David Burke
				next = data;
			}
			break;
		case DIRECTIVE_ENDM:
		case DIRECTIVE_ENDMACRO:
			print_msg(pi, MSGTYPE_ERROR, "No .MACRO found before .ENDMACRO");
			break;
		case DIRECTIVE_EQU:
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, ".EQU needs an operand");
				return(True);
			}
			data = get_next_token(next, TERM_EQUAL);
			if(!data) {
				print_msg(pi, MSGTYPE_ERROR, "%s needs an expression (e.g. .EQU BZZZT = 0x2a)", next);
				return(True);
			}
			get_next_token(data, TERM_END);
			if(!get_expr(pi, data, &i))
				return(False);
			if(test_label(pi,next,"%s have already been defined as a label")!=NULL) 
				return(True);
			if(test_variable(pi,next,"%s have already been defined as a .SET variable")!=NULL) 
				return(True);
			/* B.A. : New. Forward references allowed. But check, if everything is ok ... */
			if(pi->pass==PASS_1) { /* Pass 1 */
				if(test_constant(pi,next,"Can't redefine constant %s, use .SET instead")!=NULL) 
					return(True);
				if(def_const(pi, next, i)==False)
					return(False);
			} else { /* Pass 2 */
				int j;
				if(get_constant(pi, next, &j)==False) {   /* Defined in Pass 1 and now missing ? */
					print_msg(pi, MSGTYPE_ERROR, "Constant %s is missing in pass 2", next);
					return(False);
				}
				if(i != j) {
					print_msg(pi, MSGTYPE_ERROR, "Constant %s changed value from %d in pass1 to %d in pass 2", next,j,i);
					return(False);
				}
				/* OK. Definition is unchanged */
			}
			if((pi->pass == PASS_2) && pi->list_line && pi->list_on) {
				fprintf(pi->list_file, "          %s\n", pi->list_line);
				pi->list_line = NULL;
			}
			break;
		case DIRECTIVE_ESEG:
			fix_orglist(pi);
			pi->segment = SEGMENT_EEPROM;
			def_orglist(pi);
			if(pi->device->eeprom_size == 0) {
				print_msg(pi, MSGTYPE_ERROR, "Can't use .ESEG directive because device has no EEPROM");
			}
			break;
		case DIRECTIVE_EXIT:
			pi->fi->exit_file = True;
			break;
		/*** .include ***/
		case DIRECTIVE_INCLUDE:    
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, "Nothing to include");
				return(True);
			}
			next = term_string(pi, next);
			if((pi->pass == PASS_2) && pi->list_line && pi->list_on) {
				fprintf(pi->list_file, "          %s\n", pi->list_line);
				pi->list_line = NULL;
			}
			// Test if include is in local directory
			ok = test_include(next);
			data = NULL;
			if(!ok)
				for(incpath = GET_ARG(pi->args, ARG_INCLUDEPATH); incpath && !ok; incpath = incpath->next) {
					i = strlen(incpath->data);
					if(data)
						free(data);
					data = malloc(i + strlen(next) + 2);
					if(!data) {
						print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
						return(False);
					}
					strcpy(data, incpath->data);
					if((data[i - 1] != '\\') && (data[i - 1] != '/'))
						data[i++] = '/';
					strcpy(&data[i], next);
                    //printf("testing: %s\n", data);
					ok = test_include(data);
				}
			if(ok) {
				fi_bak = pi->fi;
				ok = parse_file(pi, data ? data : next);
				pi->fi = fi_bak;
			}
			else
				print_msg(pi, MSGTYPE_ERROR, "Cannot find include file: %s", next);
			if(data)
				free(data);
			break;
		/*** .includepath ***/
		case DIRECTIVE_INCLUDEPATH:
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, ".INCLUDEPATH needs an operand");
				return(True);
			}
			data = get_next_token(next, TERM_SPACE);
			if(data) {
				print_msg(pi, MSGTYPE_ERROR, ".INCLUDEPATH needs an operand!!!");
				get_next_token(data, TERM_END);
				if(!get_expr(pi, data, &i))
				        return(False);
			}
			next = term_string(pi, next);
			/* get arg list start pointer */
			incpath = GET_ARG(pi->args, ARG_INCLUDEPATH);
        /* search for last element */
        if(incpath == NULL) {
         	dl = malloc(sizeof(struct data_list));
         	data = malloc(strlen(next)+1);
	        if(dl && data) {
         		dl->next = NULL;
         		strcpy(data, next);
		        dl->data = data;
            SET_ARG(pi->args, ARG_INCLUDEPATH, dl);
         	}
         	else {
         		printf("Error: Unable to allocate memory\n");
        		return(False);
         	}
        }
        else
          add_arg(&incpath, next);
			break;
		case DIRECTIVE_LIST:
			if(pi->pass == PASS_2)
				if(pi->list_file)
          pi->list_on = True;
			break;
		case DIRECTIVE_LISTMAC:
			if(pi->pass == PASS_2)
				SET_ARG(pi->args, ARG_LISTMAC, True);
			break;
		case DIRECTIVE_MACRO:
			return(read_macro(pi, next));
//			break;
		case DIRECTIVE_NOLIST:
			if(pi->pass == PASS_2)
				pi->list_on = False;
			break;
		case DIRECTIVE_ORG:
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, ".ORG needs an operand");
				return(True);
			}
			get_next_token(next, TERM_END);
			if(!get_expr(pi, next, &i))
				return(False);
			fix_orglist(pi);		/* Update last segment */
			switch(pi->segment) {
				case SEGMENT_CODE:
					pi->cseg_addr = i;
					break;
				case SEGMENT_DATA:
					pi->dseg_addr = i;
					break;
				case SEGMENT_EEPROM:
					pi->eseg_addr = i;
			}
			def_orglist(pi);		/* Create new segment */
			if(pi->fi->label)
				pi->fi->label->value = i;
			if((pi->pass == PASS_2) && pi->list_line && pi->list_on) {
				fprintf(pi->list_file, "          %s\n", pi->list_line);
				pi->list_line = NULL;
			}
			break;
		case DIRECTIVE_SET:
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, ".SET needs an operand");
				return(True);
			}
			data = get_next_token(next, TERM_EQUAL);
			if(!data) {
				print_msg(pi, MSGTYPE_ERROR, "%s needs an expression (e.g. .SET BZZZT = 0x2a)", next);
				return(True);
			}
			get_next_token(data, TERM_END);
			if(!get_expr(pi, data, &i))
				return(False);

      if(test_label(pi,next,"%s have already been defined as a label")!=NULL) 
        return(True);
      if(test_constant(pi,next,"%s have already been defined as a .EQU constant")!=NULL) 
        return(True);
      return(def_var(pi, next, i));
//			break;
		case DIRECTIVE_DEFINE:
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, ".DEFINE needs an operand");
				return(True);
			}
			data = get_next_token(next, TERM_SPACE);
			if(data) {
				get_next_token(data, TERM_END);
				if(!get_expr(pi, data, &i))
				        return(False);
			}
			else
				i = 1;
      if(test_label(pi,next,"%s have already been defined as a label")!=NULL) 
        return(True);
      if(test_variable(pi,next,"%s have already been defined as a .SET variable")!=NULL) 
        return(True);
			/* B.A. : New. Forward references allowed. But check, if everything is ok ... */
			if(pi->pass==PASS_1) { /* Pass 1 */
	      if(test_constant(pi,next,"Can't redefine constant %s, use .SET instead")!=NULL) 
  	      return(True);
	      if(def_const(pi, next, i)==False)
    	      return(False);
			} else { /* Pass 2 */
				int j;
				if(get_constant(pi, next, &j)==False) {   /* Defined in Pass 1 and now missing ? */
   	      print_msg(pi, MSGTYPE_ERROR, "Constant %s is missing in pass 2", next);
  	      return(False);
				}
				if(i != j) {
   	      print_msg(pi, MSGTYPE_ERROR, "Constant %s changed value from %d in pass1 to %d in pass 2", next,j,i);
  	      return(False);
				}
				/* OK. Definition is unchanged */
			}
			if((pi->pass == PASS_2) && pi->list_line && pi->list_on) {
				fprintf(pi->list_file, "          %s\n", pi->list_line);
				pi->list_line = NULL;
			}
			break;
		case DIRECTIVE_PRAGMA:
#if 0
			may_do_something_with_pragma_someday();
#else
			// if ( !flag_no_warnings )
			print_msg(pi, MSGTYPE_MESSAGE, "PRAGMA directives currently ignored");
#endif
			break;
		case DIRECTIVE_UNDEF: // TODO
			break;
		case DIRECTIVE_IFDEF:
			if(!next)
				{
				print_msg(pi, MSGTYPE_ERROR, ".IFDEF needs an operand");
				return(True);
				}
			get_next_token(next, TERM_END);
			/* B.A. : Forward referenc is not allowed for ifdef and ifndef */
			/* Store undefined symbols in blacklist in pass1 and check, if they are still undefined in pass2 */
			if(get_symbol(pi, next, NULL)) {
#if 0
					// If it's not defined in the first pass, but was defined later
					// then it should be considered OK with regards to ifdef..endif and
					// ifndef..endif code sections. Removed this code.
				if(pi->pass==PASS_2) { /* B.A. : 'Still undefined'-test in pass 2 */
 		          if(test_blacklist(pi,next,"Forward reference (%s) not allowed in .ifdef directive")!=NULL)
					return(False);
				}
#else
				pi->conditional_depth++;
#endif
			} else {
				if(pi->pass==PASS_1) { /* B.A. : Store undefined symbols in pass 1 */
          if(def_blacklist(pi, next)==False) 
   	        return(False);
 				}
				if(!spool_conditional(pi, False))
	        return(False);
			}
			break;
		case DIRECTIVE_IFNDEF:
			if(!next)
				{
				print_msg(pi, MSGTYPE_ERROR, ".IFNDEF needs an operand");
				return(True);
				}
			get_next_token(next, TERM_END);
			/* B.A. : Forward referenc is not allowed for ifdef and ifndef */
			/* Store undefined symbols in blacklist in pass1 and check, if they are still undefined in pass2 */
			if(get_symbol(pi, next, NULL))
		        {
#if 0
				if(pi->pass==PASS_2) { /* B.A. : 'Still undefined'-test in pass 2 */
					// If it's not defined in the first pass, but was defined later
					// then it should be considered OK with regards to ifdef..endif and
					// ifndef..endif code sections. Removed this code.
 		          if(test_blacklist(pi,next,"Forward reference (%s) not allowed in .ifndef directive")!=NULL)
					return(False);
				}
				if(!spool_conditional(pi, False))
				        return(False);
#else
				pi->conditional_depth++;
#endif
				}
			else {
				if(pi->pass==PASS_1) { /* B.A. : Store undefined symbols in pass 1 */
		          if(def_blacklist(pi, next)==False) 
		   	        return(False);
 				}
				pi->conditional_depth++;
			}
			break;
		case DIRECTIVE_IF:
			if(!next)
				{
				print_msg(pi, MSGTYPE_ERROR, ".IF needs an expression");
				return(True);
				}
			get_next_token(next, TERM_END);
			if(!get_expr(pi, next, &i))
			        return(False);
			if(i)
				pi->conditional_depth++;
			else
			        {
				if(!spool_conditional(pi, False))
				        return(False);
				}
			break;
		case DIRECTIVE_ELSE:
		case DIRECTIVE_ELIF: 
		case DIRECTIVE_ELSEIF: 
		        if(!spool_conditional(pi, True))
			        return(False);
			break;
		case DIRECTIVE_ENDIF:
		        if(pi->conditional_depth == 0)
			        print_msg(pi, MSGTYPE_ERROR, "Too many .ENDIF");
			else
			        pi->conditional_depth--;
			break;
		case DIRECTIVE_MESSAGE:
			if(pi->pass == PASS_1)
				return(True);
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, "No message parameter supplied");
				return(True);
			} 
			/* B.A : Extended .MESSAGE. Now a comma separated list like in .db is possible and not only a string */
			print_msg(pi, MSGTYPE_MESSAGE_NO_LF, NULL); 	/* Prints Line Header (filename, linenumber) without trailing /n */
		    while(next) { 	/* Modified code from parse_db(). Thank you :-) */
			  data = get_next_token(next, TERM_COMMA);
				if(next[0] == '\"') { 	/* string parsing */
	              next = term_string(pi, next);
		 		  print_msg(pi, MSGTYPE_APPEND,"%s",next);
			      while(*next != '\0') {
					next++;
	    		  }
				} else {
 		          if(!get_expr(pi, next, &i)) {
			 		print_msg(pi, MSGTYPE_APPEND,"\n"); /* Add newline */
  		            return(False);
				  }
		 		  print_msg(pi, MSGTYPE_APPEND,"0x%02X",i);
	           }			
    	   	next = data;
		    }
	 		print_msg(pi, MSGTYPE_APPEND,"\n"); /* Add newline */
			break;
		case DIRECTIVE_WARNING:
			if(pi->pass == PASS_1)
				return(True);
			if(!next) {
				print_msg(pi, MSGTYPE_ERROR, "No warning string supplied");
				return(True);
			}
			next = term_string(pi, next);
			print_msg(pi, MSGTYPE_WARNING, next);
			break;
		case DIRECTIVE_ERROR:		
			if(!next) { /* B.A : Fix segfault bug if .error without parameter was used */
				print_msg(pi, MSGTYPE_ERROR, "No error string supplied");
				return(True);
		    }
			next = term_string(pi, next);		
			/* B.A. : Don't use this. It may cause segfaults if the 'next' contains printf control sequences %s,%d etc. 
			print_msg(pi, MSGTYPE_ERROR, next); 
			*/
			print_msg(pi, MSGTYPE_ERROR,"%s",next); /* B.A. : This is '%s' save :-) */
            pi->error_count = pi->max_errors;
			if(pi->pass == PASS_1)
				return(True);
			break;
	}
	return(ok);
}


int get_directive_type(char *directive) {
  int i;

  for(i = 0; i < DIRECTIVE_COUNT; i++) {
	if(!strcmp(directive, directive_list[i])) return(i);
  }
  return(-1);
}


char *term_string(struct prog_info *pi, char *string) {
  int i;

  if(string[0] != '\"') {
	  print_msg(pi, MSGTYPE_ERROR, "String must be enclosed in \"-signs");
  }
  else {
	  string++;
  }
  /* skip to the end of the string*/
  for(i = 0; (string[i] != '\"') && !((string[i] == 10) || (string[i] == 13) || (string[i] == '\0')); i++);
  if((string[i] == 10) || (string[i] == 13) || (string[i] == '\0')) {
	  print_msg(pi, MSGTYPE_ERROR, "String is missing a closing \"-sign");
  }
  string[i] = '\0'; /* and terminate it where the " was */
  return(string);
}

/* Parse data byte directive */
int parse_db(struct prog_info *pi, char *next) {
  int i;
  int count;
  char *data;
  char prev = 0;

  /* check if .db is allowed in this segment type */
  if(pi->segment == SEGMENT_DATA) {
	  print_msg(pi, MSGTYPE_ERROR, "Can't use .DB directive in data segment (.DSEG) !");
	  return(True);
  }

  count = 0;
  if(pi->pass == PASS_2 && pi->list_on) {
    if(pi->segment == SEGMENT_EEPROM)
      fprintf(pi->list_file, "E:%06X ", pi->eseg_addr);
    if(pi->segment == SEGMENT_CODE)
      fprintf(pi->list_file, "C:%06X ", pi->cseg_addr);
  }
  /* get each db token */
  while(next) {
	data = get_next_token(next, TERM_COMMA);
	/* string parsing */
	if(next[0] == '\"') {
	    next = term_string(pi, next);
	    while(*next != '\0') {
		count++;
		write_db(pi, *next, &prev, count);
        	if(pi->pass == PASS_2 && pi->list_on)
          		fprintf(pi->list_file, "%02X", (unsigned char)*next);	// B.A.: Patch for chars with bit 7 = 1 (Example: °)
		if((unsigned char)*next > 127 && pi->pass == PASS_2)
			print_msg(pi, MSGTYPE_WARNING, "Found .DB string with characters > code 127. Be careful !"); // B.A.: Print warning for codes > 127
		next++;
	    }
	}
	else {
	    if(pi->pass == PASS_2) {
		if(!get_expr(pi, next, &i))
			return(False);
		if((i < -128) || (i > 255))
			print_msg(pi, MSGTYPE_WARNING, "Value %d is out of range (-128 <= k <= 255). Will be masked", i);
        	if(pi->list_on) fprintf(pi->list_file, "%02X", i);
	    }
	    count++;
	    write_db(pi, (char)i, &prev, count);
	  }
	next = data;
  }
  if(pi->segment == SEGMENT_CODE) {
	if((count % 2) == 1) {
	  if(pi->pass == PASS_2)  {
        if(pi->list_on) fprintf(pi->list_file, "00 ; zero byte added");
		write_prog_word(pi, pi->cseg_addr, prev & 0xFF);
		print_msg(pi, MSGTYPE_WARNING, "A .DB segment with an odd number of bytes is detected. A zero byte is added.");
	  }
	  pi->cseg_addr++;
	  if(pi->pass == PASS_1) {
		pi->cseg_count++;
	  }
	}
  }
  if(pi->pass == PASS_2 && pi->list_on) {
    fprintf(pi->list_file, "\n");
    pi->list_line = NULL;
  }
  return(True);
}


void write_db(struct prog_info *pi, char byte, char *prev, int count) {
  if(pi->segment == SEGMENT_EEPROM)	{
    if(pi->pass == PASS_2) {
      write_ee_byte(pi, pi->eseg_addr, byte);
	}
	pi->eseg_addr++;
	if(pi->pass == PASS_1) {
	  pi->eseg_count++;
	} 
  }
  else { /* pi->segment == SEGMENT_CODE */
    if((count % 2) == 0) {
	  if(pi->pass == PASS_2) {
		write_prog_word(pi, pi->cseg_addr, (byte << 8) | (*prev & 0xff));
	  }
	  pi->cseg_addr++;
	  if(pi->pass == PASS_1) {
		pi->cseg_count++;
	  }
	}
	else {
	  *prev = byte;
	}
  }
}


int spool_conditional(struct prog_info *pi, int only_endif) {
	int current_depth = 0, do_next;

	if(pi->macro_line) {
      while((pi->macro_line = pi->macro_line->next)) {
		pi->macro_call->line_index++;
		if(check_conditional(pi, pi->macro_line->line, &current_depth,  &do_next, only_endif)) {
		  if(!do_next)
		  return(True);
		}
		else
		  return(False);
	  }
	  print_msg(pi, MSGTYPE_ERROR, "Found no closing .ENDIF in macro");
	}
	else {
	  while(fgets_new(pi,pi->fi->buff, LINEBUFFER_LENGTH, pi->fi->fp)) {
	  pi->fi->line_number++;
	    if(check_conditional(pi, pi->fi->buff, &current_depth,  &do_next, only_endif)) {
		  if(!do_next)
		  return(True);
		}
		else
		  return(False);
	  }
	  if(feof(pi->fi->fp)) {
		print_msg(pi, MSGTYPE_ERROR, "Found no closing .ENDIF");
		return(True);
      }
	  else {
		perror(pi->fi->include_file->name);
		return(False);
	  }
	}
	return(True);
}


int check_conditional(struct prog_info *pi, char *pbuff, int *current_depth, int *do_next, int only_endif)
{
	int i = 0;
	char *next;
	char linebuff[LINEBUFFER_LENGTH];

    strcpy(linebuff, pbuff); /* avoid cutting of the end of .elif line */

	*do_next = False;
	while(IS_HOR_SPACE(linebuff[i]) && !IS_END_OR_COMMENT(linebuff[i])) i++;
#if 0
	if(linebuff[i] == '.') {
#else
	if((linebuff[i] == '.') || (linebuff[i] == '#')){
#endif
	  i++;
	  if(!nocase_strncmp(&linebuff[i], "if", 2))
	    (*current_depth)++;
	  else 
		  if(!nocase_strncmp(&linebuff[i], "endif", 5)) {
        if(*current_depth == 0)
          return(True);
		    (*current_depth)--;
	    } else 
				if(!only_endif && (*current_depth == 0)) {
					/* B.A.  : Add ELSEIF  =  ELIF */
					if((!nocase_strncmp(&linebuff[i], "else", 4)) && (nocase_strncmp(&linebuff[i], "elseif", 6))) { 
		  			pi->conditional_depth++;
		      	return(True);
					}	else 
						if((!nocase_strncmp(&linebuff[i], "elif", 4)) || (!nocase_strncmp(&linebuff[i], "elseif", 6))) {
						  next = get_next_token(&linebuff[i], TERM_SPACE);
						  if(!next) {
						    print_msg(pi, MSGTYPE_ERROR, ".ELSEIF / .ELIF needs an operand");
							  return(True);
						  }
						  get_next_token(next, TERM_END);
						  if(!get_expr(pi, next, &i))
						    return(False);
						  if(i)
						    pi->conditional_depth++;
						  else {
								if(!spool_conditional(pi, False))
								  return(False);
						  }
						  return(True);
	    			}
	  		}
	}
	*do_next = True;
	return(True);
}

int test_include(const char *filename)
{
  FILE *fp;
  fp = fopen(filename, "r");
  if(fp)
  {
    fclose(fp);
    return(True);
  }
  else
    return(False);
}

/* end of directiv.c */


