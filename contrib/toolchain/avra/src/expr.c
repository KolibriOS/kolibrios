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
#include <ctype.h>

#include "misc.h"
#include "avra.h"
#include "device.h"

#define IS_UNARY(x) ((x == '!') || (x == '-') || (x == '~'))
#define IS_OPERATOR(x) ((x == '+') || (x == '-') || (x == '*') || (x == '/') || (x == '%') || (x == '<') || (x == '>') || (x == '=') || (x == '!') || (x == '&') || (x == '^') || (x == '|'))
#define IS_2ND_OPERATOR(x) ((x == '<') || (x == '>') || (x == '=') || (x == '&') || (x == '|'))

enum {
	OPERATOR_ERROR = 0,
	OPERATOR_MUL,
	OPERATOR_DIV,
	OPERATOR_MOD,
	OPERATOR_ADD,
	OPERATOR_SUB,
	OPERATOR_SHIFT_LEFT,
	OPERATOR_SHIFT_RIGHT,
	OPERATOR_LESS_THAN,
	OPERATOR_LESS_OR_EQUAL,
	OPERATOR_GREATER_THAN,
	OPERATOR_GREATER_OR_EQUAL,
	OPERATOR_EQUAL,
	OPERATOR_NOT_EQUAL,
	OPERATOR_BITWISE_AND,
	OPERATOR_BITWISE_XOR,
	OPERATOR_BITWISE_OR,
	OPERATOR_LOGICAL_AND,
	OPERATOR_LOGICAL_OR
	};

enum {
	FUNCTION_LOW = 0,
	FUNCTION_BYTE1,
	FUNCTION_HIGH,
	FUNCTION_BYTE2,
	FUNCTION_BYTE3,
	FUNCTION_BYTE4,
	FUNCTION_LWRD,
	FUNCTION_HWRD,
	FUNCTION_PAGE,
	FUNCTION_EXP2,
	FUNCTION_LOG2,
	FUNCTION_COUNT
};

struct element
	{
	struct element *next;
	int data;
	};

char *function_list[] = {
    /* 
    ** allow whitespace between function name
    ** and opening brace...
    */
	"low",
	"byte1",
	"high",
	"byte2",
	"byte3",
	"byte4",
	"lwrd",
	"hwrd",
	"page",
	"exp2",
	"log2"
};

int log_2(int value)
{
  int i = 0;
  while(value >>= 1)
    i++;
  return(i);
}

int get_operator(char *op)
{
	switch(op[0]) {
		case '*':
			return(OPERATOR_MUL);
		case '/':
			return(OPERATOR_DIV);
		case '%':
			return(OPERATOR_MOD);
		case '+':
			return(OPERATOR_ADD);
		case '-':
			return(OPERATOR_SUB);
		case '<':
			switch(op[1]) {
				case '<':
					return(OPERATOR_SHIFT_LEFT);
				case '=':
					return(OPERATOR_LESS_OR_EQUAL);
				default:
					return(OPERATOR_LESS_THAN);
			}
		case '>':
			switch(op[1]) {
				case '>':
					return(OPERATOR_SHIFT_RIGHT);
				case '=':
					return(OPERATOR_GREATER_OR_EQUAL);
				default:
					return(OPERATOR_GREATER_THAN);
			}
		case '=':
			if(op[1] == '=')
				return(OPERATOR_EQUAL);
		case '!':
			if(op[1] == '=')
				return(OPERATOR_NOT_EQUAL);
		case '&':
			if(op[1] == '&')
				return(OPERATOR_LOGICAL_AND);
			else
				return(OPERATOR_BITWISE_AND);
		case '^':
			return(OPERATOR_BITWISE_XOR);
		case '|':
			if(op[1] == '|')
				return(OPERATOR_LOGICAL_OR);
			else
				return(OPERATOR_BITWISE_OR);
	}
	return(OPERATOR_ERROR);
}




int test_operator_at_precedence(int operator, int precedence)
{
	switch(precedence) {
		case 13:
			return((operator == OPERATOR_MUL) || (operator == OPERATOR_DIV)
			       || (operator == OPERATOR_MOD));
		case 12:
			return((operator == OPERATOR_ADD) || (operator == OPERATOR_SUB));
		case 11:
			return((operator == OPERATOR_SHIFT_LEFT) || (operator == OPERATOR_SHIFT_RIGHT));
		case 10:
			return((operator == OPERATOR_LESS_THAN) || (operator == OPERATOR_LESS_OR_EQUAL)
			       || (operator == OPERATOR_GREATER_THAN) || (operator == OPERATOR_GREATER_OR_EQUAL));
		case 9:
			return((operator == OPERATOR_EQUAL) || (operator == OPERATOR_NOT_EQUAL));
		case 8:
			return(operator == OPERATOR_BITWISE_AND);
		case 7:
			return(operator == OPERATOR_BITWISE_XOR);
		case 6:
			return(operator == OPERATOR_BITWISE_OR);
		case 5:
			return(operator == OPERATOR_LOGICAL_AND);
		default: /* Makes the compiler shut up */
		case 4:
			return(operator == OPERATOR_LOGICAL_OR);
	}
}


int calc(struct prog_info *pi, int left, int operator, int right) // TODO: Sjekk litt resultater
{
	switch(operator) {
		case OPERATOR_MUL:
			return(left * right);
		case OPERATOR_DIV:
			if(right == 0) {
				print_msg(pi, MSGTYPE_ERROR, "Division by zero");
				return(0);
			}
			return(left / right);
		case OPERATOR_MOD:
			if(right == 0) {
				print_msg(pi, MSGTYPE_ERROR, "Division by zero (modulus operator)");
				return(0);
			}
		    return(left % right);
		case OPERATOR_ADD:
			return(left + right);
		case OPERATOR_SUB:
			return(left - right);
		case OPERATOR_SHIFT_LEFT:
			return(left << right);
		case OPERATOR_SHIFT_RIGHT:
			return((unsigned)left >> right);
		case OPERATOR_LESS_THAN:
			return(left < right);
		case OPERATOR_LESS_OR_EQUAL:
			return(left <= right);
		case OPERATOR_GREATER_THAN:
			return(left > right);
		case OPERATOR_GREATER_OR_EQUAL:
			return(left >= right);
		case OPERATOR_EQUAL:
			return(left == right);
		case OPERATOR_NOT_EQUAL:
			return(left != right);
		case OPERATOR_BITWISE_AND:
			return(left & right);
		case OPERATOR_BITWISE_XOR:
			return(left ^ right);
		case OPERATOR_BITWISE_OR:
			return(left | right);
		case OPERATOR_LOGICAL_AND:
			return(left && right);
		default: /* Make the compiler shut up */
		case OPERATOR_LOGICAL_OR:
			return(left || right);
	}
}

/* If found, return the ID of the internal function */
int get_function(char *function)
{
	int i;

	for(i = 0; i < FUNCTION_COUNT; i++) {
		if(!nocase_strncmp(function, function_list[i], strlen(function_list[i])))
        {
            /*
            ** some more checks to allow whitespace between function name
            ** and opening brace...
            */
            char *tmp = function + strlen(function_list[i]);
            while (*tmp <= ' ')
                tmp++;
            if (*tmp != '(')
                continue;

      return(i);
  }  
    }  
	return(-1);
}

unsigned int do_function(int function, int value)
{
	switch(function) {
		case FUNCTION_LOW:
		case FUNCTION_BYTE1:
			return(value & 0xFF);
		case FUNCTION_HIGH:
		case FUNCTION_BYTE2:
			return((value >> 8) & 0xff);
		case FUNCTION_BYTE3:
			return((value >> 16) & 0xff);
		case FUNCTION_BYTE4:
			return((value >> 24) & 0xff);
		case FUNCTION_LWRD:
			return(value & 0xffff);
		case FUNCTION_HWRD:
			return((value >> 16) & 0xffff);
		case FUNCTION_PAGE:
			return((value >> 16) & 0xff);
		case FUNCTION_EXP2:
			return(1 << value);
		case FUNCTION_LOG2:
			return(log_2(value));
		default:
			return(0);
	}
}


int get_symbol(struct prog_info *pi, char *label_name, int *data)
{
	struct label *label;
	struct macro_call *macro_call;

	if(get_constant(pi,label_name,data)) return(True);
	if(get_variable(pi,label_name,data)) return(True);

	for(macro_call = pi->macro_call; macro_call; macro_call = macro_call->prev_on_stack) {
		for(label = pi->macro_call->first_label; label; label = label->next)
			if(!nocase_strcmp(label->name, label_name)) {
				if(data)
					*data = label->value;
				return(True);
			}
	}

	if(get_label(pi,label_name,data)) return(True);
	return(False);
}


int par_length(char *data)
{
	int i = 0, b_count = 1;

	for(;;) {
		if(data[i] == ')') {
			b_count--;
			if(!b_count)
				return(i);
		}
		else if(data[i] == '(')
			b_count++;
		else if(data[i] == '\0')
			return(-1);
		i++;
	}
}

int get_expr(struct prog_info *pi, char *data, int *value) {
  /* Definition */
	int ok, end, i, count, first_flag, length, function;
	char unary, *label;
	struct element *element, *first_element = NULL, *temp_element;
	struct element **last_element = &first_element;

  /* Initialisation */
  first_flag  = True;
  ok          = True;
  end         = False;
  count       = 0;
  unary       = 0;
  /* the expression parser loop */
	for(i = 0; ; i++) {
	  /* horizontal space is just skipped */
		if(IS_HOR_SPACE(data[i]));
		/* test for clean or premature end */
		else if(IS_END_OR_COMMENT(data[i])) {
			if((count % 2) != 1)
				print_msg(pi, MSGTYPE_ERROR, "Missing value in expression");
			else
				end = True;
			break;
		}
		else if(first_flag && IS_UNARY(data[i])) {
			unary = data[i];
			first_flag = False;
		}
		else if((count % 2) == 1) {
			if(!IS_OPERATOR(data[i])) {
				print_msg(pi, MSGTYPE_ERROR, "Illegal operator '%c'", data[i]);
				break;
			}
			element = malloc(sizeof(struct element));
			if(!element) {
				print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
				ok = False;
				break;
			}
			element->next = NULL;
			element->data = get_operator(&data[i]);
			if(element->data == OPERATOR_ERROR) {
				if(IS_2ND_OPERATOR(data[i + 1]))
					print_msg(pi, MSGTYPE_ERROR, "Unknown operator %c%c", data[i], data[i + 1]);
				else
					print_msg(pi, MSGTYPE_ERROR, "Unknown operator %c", data[i]);
				break;
			}
			*last_element = element;
			last_element = &element->next;
			if(IS_2ND_OPERATOR(data[i + 1]))
				i++;
			count++;
			first_flag = True;
			unary = 0;
		}
		else {
			element = malloc(sizeof(struct element));
			if(!element) {
				print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
				ok = False;
				break;
			}
			element->next = NULL;
			length = 0;
			if(isdigit(data[i])) {
				if(tolower(data[i + 1]) == 'x') {
					i += 2;
					while(isxdigit(data[i + length])) length++; // TODO: Sjekk overflow
					element->data = atox_n(&data[i], length);
				}
				else if(tolower(data[i + 1]) == 'b') {
					i += 2;
					element->data = 0;
					while((data[i + length] == '1') || (data[i + length] == '0')) {
						element->data <<= 1;
						element->data |= data[i + length++] - '0'; // TODO: Sjekk overflow
					}
				}
				else {
					while(isdigit(data[i + length])) length++;
					  element->data = atoi_n(&data[i], length); // TODO: Sjekk overflow
				}
			}
			else if(data[i] == '$') {
				i++;
				while(isxdigit(data[i + length])) length++;
				element->data = atox_n(&data[i], length); // TODO: Sjekk overflow
			}
			else if(data[i] == '\'') {
				i++;
				if(data[i+1] != '\'') {
					print_msg(pi, MSGTYPE_ERROR, "Not a correct character ! Use 'A' !");
					break;
				}
				element->data = data[i];
				length = 2;
			}
			else if(data[i] == '(') {
				i++;
				length = par_length(&data[i]);
				if(length == -1) {
					print_msg(pi, MSGTYPE_ERROR, "Missing ')'");
					break;
				}
				data[i + length++] = '\0';
				ok = get_expr(pi, &data[i], &element->data);
				if(!ok)
					break;
			}
			/* test for internal function */
			else if((function = get_function(&data[i])) != -1) {
				while(data[i] != '(')
					i++;
				i++;
				length = par_length(&data[i]);
				if(length == -1) {
					print_msg(pi, MSGTYPE_ERROR, "Missing ')'");
					break;
				}
				data[i + length++] = '\0';
				ok = get_expr(pi, &data[i], &element->data);
				if(!ok)
					break;
				element->data = do_function(function, element->data);
			}
			else if(!nocase_strncmp(&data[i], "defined(", 8)) {
				i += 8;
				length = par_length(&data[i]);
				if(length == -1) {
					print_msg(pi, MSGTYPE_ERROR, "Missing ')'");
					break;
				}
				data[i + length++] = '\0';
				if(get_symbol(pi, &data[i], NULL))
					element->data = 1;
				else
					element->data = 0;
			}
			else if(!nocase_strncmp(&data[i], "supported(", 10)) {
				i += 10;
				length = par_length(&data[i]);
				if(length == -1) {
					print_msg(pi, MSGTYPE_ERROR, "Missing ')'");
					break;
				}
				data[i + length++] = '\0';
				element->data=is_supported(pi, &data[i]);
				if (element->data<0) {
					if (toupper(data[i])=='X') {
						if (pi->device->flag&DF_NO_XREG) element->data = 0;
						else element->data = 1;
					}
					else if (toupper(data[i])=='Y') {
						if (pi->device->flag&DF_NO_YREG) element->data = 0;
						else element->data = 1;
					}
					else if (toupper(data[i])=='Z')
						element->data = 1;
					else {
						print_msg(pi, MSGTYPE_ERROR, "Unknown mnemonic: %s",&data[i]);
						element->data = 0;
					}
				}
			}
			else {
				while(IS_LABEL(data[i + length])) length++;
				if((length == 2) && !nocase_strncmp(&data[i], "PC", 2))
					element->data = pi->cseg_addr;
				else {
					label = malloc(length + 1);
					if(!label) {
						print_msg(pi, MSGTYPE_OUT_OF_MEM, NULL);
						ok = False;
						break;
					}
					strncpy(label, &data[i], length);
					label[length] = '\0';
					if(get_symbol(pi, label, &element->data))
						free(label);
					else {
						print_msg(pi, MSGTYPE_ERROR, "Found no label/variable/constant named %s", label);
						free(label);
						break;
					}
				}
			}
			/* now the expression has been evaluated */
			i += length - 1;
			switch(unary) { // TODO: Få den til å takle flere unary på rad.
				case '-':
					element->data = -element->data;
					break;
				case '!':
					element->data = !element->data;
					break;
				case '~':
					element->data = ~element->data;
			}
			*last_element = element;
			last_element = &element->next;
			count++;
			first_flag = False;
		}
	}
	if(end) {
		for(i = 13; (i >= 4) && (count != 1); i--) {
			for(element = first_element; element->next;) {
				if(test_operator_at_precedence(element->next->data, i)) { // TODO: Vurder en hi_i for kjapphet
					element->data = calc(pi, element->data, element->next->data, element->next->next->data);
					temp_element = element->next->next->next;
					free(element->next->next);
					free(element->next);
					count -= 2;
					element->next = temp_element;
				}
				else
					element = element->next->next;
			}
		}
		*value = first_element->data;
	}
	for(element = first_element; element;) {
		temp_element = element;
		element = element->next;
		free(temp_element);
	}
	return(ok);
}


/* end of expr.c */

