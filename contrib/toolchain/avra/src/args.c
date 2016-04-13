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

#include "misc.h"
#include "args.h"


struct args *alloc_args(int arg_count)
{
	struct args *args;

	args = malloc(sizeof(struct args));
	if(args) {
		args->arg = malloc(sizeof(struct arg) * arg_count);
		if(args->arg) {
			args->count = arg_count;
			args->first_data = NULL;
			return(args);
		}
		free(args);
	}
	printf("Error: Unable to allocate memory\n");
	return(NULL);
}


int read_args(struct args *args, int argc, char *argv[])
{
	int i, j, k, ok, i_old;
	struct data_list **last_data;
	/*** init ***/
	ok = True;
	args->first_data = NULL;
	/*** end of init ***/

	last_data = &args->first_data;

	for(i = 1; (i < argc) && ok; i++) {
		if(argv[i][0] == '-') {
			last_data = &args->first_data;
			if(argv[i][1] == 0) {
				printf("Error: Unknown option: -\n");
				ok = False;
			} else 
				if(argv[i][1] == '-') {
					j = 0;
					while((j != args->count) && strcmp(&argv[i][2], args->arg[j].longarg)) {
						j++;
					}
					if(j == args->count) {
						printf("Error: Unknown option: %s\n", argv[i]);
						ok = False;
					} else {
						switch(args->arg[j].type) {
							case ARGTYPE_STRING:
							case ARGTYPE_STRING_MULTISINGLE:
								/* if argument is a string parameter we will do this: */
								if((i + 1) == argc) {
									printf("Error: No argument supplied with option: %s\n", argv[i]);
									ok = False;
								} else 
									if(args->arg[j].type != ARGTYPE_STRING_MULTISINGLE)
										args->arg[j].data = argv[++i];
									else
										ok = add_arg((struct data_list **)&args->arg[j].data, argv[++i]);
								break;
							case ARGTYPE_BOOLEAN:
								args->arg[j].data = (char *)True;
								break;
							case ARGTYPE_STRING_MULTI:
								last_data = (struct data_list **)&args->arg[j].data;
								break;
						}
					}
				} else {
					for(k = 1, i_old = i; (argv[i][k] != '\0') && ok && (i == i_old); k++) {
						j = 0;
						while((j != args->count) && (argv[i][k] != args->arg[j].letter))
							j++;
						if(j == args->count) {
							printf("Error: Unknown option: -%c\n", argv[i][k]);
							ok = False;
						} else {
							switch(args->arg[j].type) {
								case ARGTYPE_STRING:
								case ARGTYPE_STRING_MULTISINGLE:
									if(argv[i][k + 1] != '\0') {
										printf("Error: Option -%c must be followed by it's argument\n", argv[i][k]);
										ok = False;
									} else {
										if((i + 1) == argc) {
											printf("Error: No argument supplied with option: -%c\n", argv[i][k]);
											ok = False;
										} else 
											if(args->arg[j].type != ARGTYPE_STRING_MULTISINGLE)
												args->arg[j].data = argv[++i];
											else
												ok = add_arg((struct data_list **)&args->arg[j].data, argv[++i]);
									}
									break;
								case ARGTYPE_BOOLEAN:
									args->arg[j].data = (char *)True;
									break;
								case ARGTYPE_STRING_MULTI:
									last_data = (struct data_list **)&args->arg[j].data;
									break;
								/* Parameters that have only one char attached */
								case ARGTYPE_CHAR_ATTACHED:
									if((i + 1) == argc) {
										printf("Error: missing arguments: asm file");
										ok = False;
									} else {
										switch(argv[i][++k]) {
											case 'O':
												args->arg[j].data = (char *)AVRSTUDIO;
												break;
											case 'G':
												args->arg[j].data = (char *)GENERIC;
												break;
											case 'I':
												args->arg[j].data = (char *)INTEL;
												break;
											case 'M':
												args->arg[j].data = (char *)MOTOROLA;
												break;
											default: 
												printf("Error: wrong file type '%c'",argv[i][2]);
												ok = False;
										}
									}
							}
						}
					}
				}
		} else
			ok = add_arg(last_data, argv[i]);
	}
	return(ok);
}


int add_arg(struct data_list **last_data, char *argv)
{
	struct data_list *data;

	while(*last_data)
	        last_data = &((*last_data)->next);

	data = malloc(sizeof(struct data_list));
	if(data) {
		data->next = NULL;
		data->data = argv;
		*last_data = data;
		last_data = &data->next;
	} else {
		printf("Error: Unable to allocate memory\n");
		return(False);
	}
	return(True);
}


void free_args(struct args *args)
{
	int i;
	struct data_list *data, *temp;

	for(data = args->first_data; data;) {
		temp = data;
		data = data->next;
		free(temp);
	}
	for(i = 0; i != args->count; i++)
		if((args->arg[i].type == ARGTYPE_STRING_MULTI)
		   || (args->arg[i].type == ARGTYPE_STRING_MULTISINGLE))
			for(data = args->arg[i].data; data;) {
				temp = data;
				data = data->next;
				free(temp);
			}
	free(args);
}


void define_arg(struct args *args, int index, int type, char letter, char *longarg, void *def_value)
{
	args->arg[index].type = type;
	args->arg[index].letter = letter;
	args->arg[index].longarg = longarg;
	args->arg[index].data = def_value;
}

/* end of args.c */

