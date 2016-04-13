/***********************************************************************
 *
 *  avra - Assembler for the Atmel AVR microcontroller series
 *
 *  Copyright (C) 1998-2004 Jon Anders Haugum, TObias Weber
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

/*
 * In append_type: added generic register names support
 * Alexey Pavluchenko, 16.Nov.2005
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "misc.h"
#include "args.h"
#include "avra.h"
#include "device.h"

/* Only Windows LIBC does support itoa, so we add this
   function for other systems here manually. Thank you
   Peter Hettkamp for your work. */

#ifndef WIN32
char * itoa(int num, char *str, const int number_format)
{
        int num1 = num;
        int num_chars = 0;
        int pos;
        
        while (num1>0)
        {
                num_chars++;
                num1 /= number_format;
        }

        if (num_chars == 0) num_chars = 1;

        str[num_chars] = 0;
        
        for (pos = num_chars-1; pos>=0; pos--)
        {
                int cur_char = num % number_format;
                
                if (cur_char < 10) /* Insert number */
                {
                        str[pos] = cur_char + '0';
                }
                else
                {
                        str[pos] = cur_char-10 + 'A';
                }
                
                num /= number_format;
        }
	return(str);
}
#endif


int read_macro(struct prog_info *pi, char *name) 
{
	int loopok;
    int i;
    int start;
	struct macro *macro;
	struct macro_line *macro_line;
    struct macro_line **last_macro_line = NULL;
	struct macro_label *macro_label;

	if(pi->pass == PASS_1) {
		if(!name) {
			print_msg(pi, MSGTYPE_ERROR, "missing macro name");
			return(True);
		}
		get_next_token(name, TERM_END);

		for(i = 0; !IS_END_OR_COMMENT(name[i]); i++) {	
			if(!IS_LABEL(name[i])) {
				print_msg(pi, MSGTYPE_ERROR, "illegal characters used in macro name '%s'",name);
				return(False);
			}
		}

		macro = calloc(1, sizeof(struct macro));
		if(!macro) {
			print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
			return(False);
		}

  		if(pi->last_macro)
			pi->last_macro->next = macro;
		else
			pi->first_macro = macro;
		pi->last_macro = macro;
		macro->name = malloc(strlen(name) + 1);
		if(!macro->name) {
			print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
			return(False);
		}
		strcpy(macro->name, name);
		macro->include_file = pi->fi->include_file;
		macro->first_line_number = pi->fi->line_number;
		last_macro_line = &macro->first_macro_line;
	}
	else {  /* pi->pass == PASS_2 */
		if(pi->list_line && pi->list_on) {
			fprintf(pi->list_file, "          %s\n", pi->list_line);
			pi->list_line = NULL;
		}
		// reset macro label running numbers
		get_next_token(name, TERM_END);
		macro = get_macro(pi, name);
		if (!macro) {
			print_msg(pi, MSGTYPE_ERROR, "macro inconsistency in '%s'", name);
			return(True);
		}
		for(macro_label = macro->first_label; macro_label; macro_label = macro_label->next) {
			macro_label->running_number = 0;
		}
	}

	loopok = True;
	while(loopok) {
		if(fgets_new(pi,pi->fi->buff, LINEBUFFER_LENGTH, pi->fi->fp)) {
			pi->fi->line_number++;
			i = 0;
			while(IS_HOR_SPACE(pi->fi->buff[i]) && !IS_END_OR_COMMENT(pi->fi->buff[i])) i++;
			if(pi->fi->buff[i] == '.') {
			  i++;
			  if(!nocase_strncmp(&pi->fi->buff[i], "endm", 4))
				loopok = False;
			  if(!nocase_strncmp(&pi->fi->buff[i], "endmacro", 8))
				loopok = False;
			}
			if(pi->pass == PASS_1) {
				if(loopok) {
					i = 0; /* find start of line */
					while(IS_HOR_SPACE(pi->fi->buff[i]) && !IS_END_OR_COMMENT(pi->fi->buff[i])) {
     					i++;
					}
					start = i;
					/* find end of line */
					while(!IS_END_OR_COMMENT(pi->fi->buff[i]) && (IS_LABEL(pi->fi->buff[i]) || pi->fi->buff[i] == ':')) {
     					i++;
					}
					if(pi->fi->buff[i-1] == ':' && (pi->fi->buff[i-2] == '%' 
     					&& (IS_HOR_SPACE(pi->fi->buff[i]) || IS_END_OR_COMMENT(pi->fi->buff[i])))) {
						if(macro->first_label) {
							for(macro_label = macro->first_label; macro_label->next; macro_label=macro_label->next){}
             				macro_label->next = calloc(1,sizeof(struct macro_label));
             				macro_label = macro_label->next;
		   				}
         				else {
             				macro_label = calloc(1,sizeof(struct macro_label));
             				macro->first_label = macro_label;
                		}
                		macro_label->label = malloc(strlen(&pi->fi->buff[start])+1);
                		pi->fi->buff[i-1] = '\0';
           				strcpy(macro_label->label, &pi->fi->buff[start]);
                		pi->fi->buff[i-1] = ':';
           				macro_label->running_number = 0;
					}
				
					macro_line = calloc(1, sizeof(struct macro_line));
					if(!macro_line) {
						print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
						return(False);
					}
					*last_macro_line = macro_line;
					last_macro_line = &macro_line->next;
					macro_line->line = malloc(strlen(pi->fi->buff) + 1);
					if(!macro_line->line) {
						print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
						return(False);
					}
					strcpy(macro_line->line, &pi->fi->buff[start]);
				}
			}
			else if(pi->fi->buff && pi->list_file && pi->list_on) {
				if(pi->fi->buff[i] == ';')
					fprintf(pi->list_file, "         %s\n", pi->fi->buff);
				else
					fprintf(pi->list_file, "          %s\n", pi->fi->buff);
			}
		}
		else {
			if(feof(pi->fi->fp)) {
				print_msg(pi, MSGTYPE_ERROR, "Found no closing .ENDMACRO");
				return(True);
			}
			else {
				perror(pi->fi->include_file->name);
				return(False);
			}
		}
	}
	return(True);
}


struct macro *get_macro(struct prog_info *pi, char *name)
{
	struct macro *macro;

	for(macro = pi->first_macro; macro; macro = macro->next)	
		if(!nocase_strcmp(macro->name, name))
			return(macro);
	return(NULL);
}

void append_type(struct prog_info *pi, char *name, int c, char *value)
{
	int p, l;
	struct def *def;

	p = strlen(name);
	name[p++] = '_';

	if(c == 0)
	{
		name[p++] = 'v';
		name[p] = '\0';
		return;
	}

	l = strlen(value);
	if ((l==2 || l==3) && (tolower(value[0])=='r') && isdigit(value[1]) && (l==3?isdigit(value[2]):1) && (atoi(&value[1])<32))
	{
		itoa((c*8),&name[p],10);
		return;
	}
			

	for(def = pi->first_def; def; def = def->next)
		if(!nocase_strcmp(def->name, value))
		{
			itoa((c*8),&name[p],10);
			return;
		}

	name[p++] = 'i';
	name[p] = '\0';
}


/*********************************************************
 * This routine replaces the macro call with mnemonics.  *
 *********************************************************/

int expand_macro(struct prog_info *pi, struct macro *macro, char *rest_line)
{
  int 	ok = True, macro_arg_count = 0, off, a, b = 0, c, i = 0, j = 0; 
  char 	*line = NULL;
  char  *temp;
  char  *macro_args[MAX_MACRO_ARGS];
  char  tmp[7];
  char 	buff[LINEBUFFER_LENGTH];
  char	arg = False; 
  char	*nmn; //string buffer for 'n'ew 'm'acro 'n'ame
  struct 	macro_line *old_macro_line;
  struct 	macro_call *macro_call;
  struct	macro_label *macro_label;

  if(rest_line) {
    //we reserve some extra space for extended macro parameters
    line = malloc(strlen(rest_line) + 20); 
 	if(!line) {
	  print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
	  return(False);
    }
		
	/* exchange amca word 'src' with YH:YL and 'dst' with ZH:ZL */
	for(c = 0, a = strlen(rest_line); c < a; c++) {
	  switch (tolower(rest_line[c])) {
		case 's':
          if(IS_SEPARATOR(rest_line[c-1]) && (rest_line[c+1] == 'r') && (rest_line[c+2] == 'c') && IS_SEPARATOR(rest_line[c+3])) {
            strcpy(&line[b],"YH:YL");
            b += 5;
            c += 2;
          }
          else {
			line[b++] = rest_line[c];
		  }
          break;
        case 'd':
          if(IS_SEPARATOR(rest_line[c-1]) && (rest_line[c+1] == 's') && (rest_line[c+2] == 't') && IS_SEPARATOR(rest_line[c+3])) {
            strcpy(&line[b],"ZH:ZL");
            b += 5;
            c += 2;
		  }
          else {
			line[b++] = rest_line[c];
		  } 
          break;
//        case ';':
//          break;
        default:
          line[b++] = rest_line[c];                
	  }
	}
    strcpy(&line[b],"\n"); /* set CR/LF at the end of the line */
		
	
	/*  here we split up the macro arguments into "macro_args"
	 *  Extended macro code interpreter added by TW 2002
	 */
		
	temp = line;
    /* test for advanced parameters */
	if( temp[0] == '[' ) { // there must be "[" " then "]", else it is garbage
      if(!strchr(temp, ']')) {
     	print_msg(pi, MSGTYPE_ERROR, "found no closing ']'");
		return(False);
	  }
  
      // Okay now we are within the advanced code interpreter
  	
	  temp++; // = &temp[1]; // skip the first bracket
	  nmn = malloc(LINEBUFFER_LENGTH);
      if(!nmn) {
	    print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
	    return(False);
	  }
	  strcpy(nmn,macro->name); // create a new macro name buffer
	  c = 1; // byte counter
	  arg = True; // loop flag

	  while(arg) {
		while(IS_HOR_SPACE(temp[0])) { //skip leading spaces
    	  temp++; // = &temp[1];
		}
		off = 0; // pointer offset
		do {
		  switch(temp[off]) { //test current character code
			case ':':
    		  temp[off] = '\0';
			  if(off > 0) {
				c++;
       			macro_args[macro_arg_count++] = temp;
			  }
   			  else {
				print_msg(pi, MSGTYPE_ERROR, "missing register before ':'",nmn);
				return(False);
			  }
   			  break;
			case ']':
 			  arg = False;
    		case ',':
			  a = off;
			  do temp[a--] = '\0'; while( IS_HOR_SPACE(temp[a]) );
      		  if(off > 0) {
       			macro_args[macro_arg_count++] = temp;
				append_type(pi, nmn, c, temp);
				c = 1;
			  }
   			  else {
				append_type(pi, nmn, 0, temp);
				c = 1;
			  } 
			  break;

       		 default:
       		  off++;
		  }
		}
		while(temp[off] != '\0');

		if(arg) temp = &temp[off+1];
	 	else break;
	  }

	  macro = get_macro(pi,nmn);
	  if(macro == NULL) {
	    print_msg(pi, MSGTYPE_ERROR, "Macro %s is not defined !",nmn);
	    return(False);
	  }
      free(nmn);
	}
    /* or else, we handle the macro as normal macro */
    else {
      line = malloc(strlen(rest_line) + 1);
      if(!line) {
        print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
        return(False);
      }
      strcpy(line, rest_line);
      temp = line;
      while(temp) {
        macro_args[macro_arg_count++] = temp;
        temp = get_next_token(temp, TERM_COMMA);
      }
	}
  }

  if(pi->pass == PASS_1) {
	macro_call = calloc(1, sizeof(struct macro_call));
	if(!macro_call) {
	  print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
	  return(False);
	}
	if(pi->last_macro_call)
	  pi->last_macro_call->next = macro_call;
	else
	  pi->first_macro_call = macro_call;
		
	pi->last_macro_call = macro_call;
	macro_call->line_number = pi->fi->line_number;
	macro_call->include_file = pi->fi->include_file;
	macro_call->macro = macro;
	macro_call->prev_on_stack = pi->macro_call;
		
  	if(macro_call->prev_on_stack) {
  	  macro_call->nest_level = macro_call->prev_on_stack->nest_level + 1;
  	  macro_call->prev_line_index = macro_call->prev_on_stack->line_index;
	}
  }
  else {
	for(macro_call = pi->first_macro_call; macro_call; macro_call = macro_call->next) {
	  if((macro_call->include_file->num == pi->fi->include_file->num) && (macro_call->line_number == pi->fi->line_number)) {
		if(pi->macro_call) {
		/* Find correct macro_call when using recursion and nesting */
		  if(macro_call->prev_on_stack == pi->macro_call)
			if((macro_call->nest_level == (pi->macro_call->nest_level + 1)) && (macro_call->prev_line_index == pi->macro_call->line_index))
			  break;
		}
		else break;
	  }
	}
	if(pi->list_line && pi->list_on) {
	  fprintf(pi->list_file, "C:%06x   +  %s\n", pi->cseg_addr, pi->list_line);
	  pi->list_line = NULL;
	}
  }
  
  macro_call->line_index = 0;
  pi->macro_call = macro_call;
  old_macro_line = pi->macro_line;
		
  //printf("\nconvert macro: '%s'\n",macro->name);

  for(pi->macro_line = macro->first_macro_line; pi->macro_line && ok; pi->macro_line = pi->macro_line->next) {
    macro_call->line_index++;
	if(GET_ARG(pi->args, ARG_LISTMAC))
	  pi->list_line = buff;
	else
	  pi->list_line = NULL;
				
	/* here we change jumps/calls within macro that corresponds to macro labels.
   	   Only in case there is an entry in macro_label list */
   
    strcpy(buff,"\0");
    macro_label = get_macro_label(pi->macro_line->line,macro);
	if(macro_label)	{
      /* test if the right macro label has been found */
	  temp = strstr(pi->macro_line->line,macro_label->label);
      c = strlen(macro_label->label);
      if(temp[c] == ':') { /* it is a label definition */
      	macro_label->running_number++;
      	strncpy(buff, macro_label->label, c - 1);
		buff[c - 1] = 0;
        i = strlen(buff) + 2; /* we set the process indeafter label */
        /* add running number to it */
		strcpy(&buff[c-1],itoa(macro_label->running_number, tmp, 10));
		strcat(buff, ":\0");
	  }
      else if(IS_HOR_SPACE(temp[c]) || IS_END_OR_COMMENT(temp[c]))	{ /* it is a jump to a macro defined label */
      	strcpy(buff,pi->macro_line->line);
      	temp = strstr(buff, macro_label->label);
      	i = temp - buff + strlen(macro_label->label);
        strncpy(temp, macro_label->label, c - 1);
      	strcpy(&temp[c-1], itoa(macro_label->running_number, tmp, 10));
	  }
	}
   	else {
      i = 0;
	}

	/* here we check every character of current line */
	for(j = i; pi->macro_line->line[i] != '\0'; i++) {
	  /* check for register place holders */
	  if(pi->macro_line->line[i] == '@') {
  		i++;
  		if(!isdigit(pi->macro_line->line[i]))
		  print_msg(pi, MSGTYPE_ERROR, "@ must be followed by a number");
        else if((pi->macro_line->line[i] - '0') >= macro_arg_count)
          print_msg(pi, MSGTYPE_ERROR, "Missing macro argument (for @%c)", pi->macro_line->line[i]);
        else {
          /* and replace them with given registers */
          strcat(&buff[j], macro_args[pi->macro_line->line[i] - '0']);
          j += strlen(macro_args[pi->macro_line->line[i] - '0']);
		}
	  }
      else if (pi->macro_line->line[i] == ';') {
        strncat(buff, "\n", 1);
        break;
      }
      else {
        strncat(buff, &pi->macro_line->line[i], 1);
	  }
	}
    
    ok = parse_line(pi, buff);
    if(ok) {
	  if((pi->pass == PASS_2) && pi->list_line && pi->list_on)
	    fprintf(pi->list_file, "         %s\n", pi->list_line);
	  if(pi->error_count >= pi->max_errors) {
	    print_msg(pi, MSGTYPE_MESSAGE, "Maximum error count reached. Exiting...");
	    ok = False;
	  break;
	  }
	} 
  }

  pi->macro_line = old_macro_line;
  pi->macro_call = macro_call->prev_on_stack;
  if(rest_line)
    free(line);
  return(ok);
}

struct macro_label *get_macro_label(char *line, struct macro *macro)
{
	char *temp ;
 	struct macro_label *macro_label;
	
	for(macro_label = macro->first_label; macro_label; macro_label = macro_label->next) {
	    temp = strstr(line,macro_label->label);
		if(temp) {
			return macro_label;
		}
	}
	return NULL;
}

/* end of macro.c */

