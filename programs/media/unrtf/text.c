
/*=============================================================================
   GNU UnRTF, a command-line program to convert RTF documents to other formats.
   Copyright (C) 2000,2001 Zachary Thayer Smith

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   The author is reachable by electronic mail at tuorfa@yahoo.com.
=============================================================================*/


/*----------------------------------------------------------------------
 * Module name:    text
 * Author name:    Zach Smith
 * Create date:    19 Sep 01
 * Purpose:        Plain text output module
 *----------------------------------------------------------------------
 * Changes:
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks 
 *--------------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include "malloc.h"
#include "defs.h"
#include "error.h"
#include "main.h"
#include "output.h"


static char*
ascii_translation_table [96] = {
	/* 0x20 */ " ", "!", "\"", "#", "$", "%", "&", "'", 
	/* 0x28 */ "(", ")", "*", "+", ",", "-", ".", "/", 
	/* 0x30 */ "0", "1", "2", "3", "4", "5", "6", "7", 
	/* 0x38 */ "8", "9", ":", ";", "<", "=", ">", "?", 
	/* 0x40 */ "@", "A", "B", "C", "D", "E", "F", "G", 
	/* 0x48 */ "H", "I", "J", "K", "L", "M", "N", "O", 
	/* 0x50 */ "P", "Q", "R", "S", "T", "U", "V", "W", 
	/* 0x58 */ "X", "Y", "Z", "[", "\\", "]", "^", "_", 
	/* 0x60 */ "`", "a", "b", "c", "d", "e", "f", "g", 
	/* 0x68 */ "h", "i", "j", "k", "l", "m", "n", "o", 
	/* 0x70 */ "p", "q", "r", "s", "t", "u", "v", "w", 
	/* 0x78 */ "x", "y", "z", "{", "|", "}", "~", "", 
};

static char*
upper_translation_table [128] = {
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
	"?", "?", "?", "?", "?", "?", "?", "?", 
};



/*========================================================================
 * Name:	text_init
 * Purpose:	Generates an output personality for the plain text format.
 * Args:	None.
 * Returns:	OutputPersonality.
 *=======================================================================*/

OutputPersonality *
text_init (void) 
{
	OutputPersonality* text_op;

	text_op = op_create();

	text_op->comment_begin = "### ";
	text_op->comment_end = "\n";

	text_op->document_begin = "";
	text_op->document_end = "";

	text_op->header_begin = "";
	text_op->header_end = "";

	text_op->document_title_begin = "\nTITLE: ";
	text_op->document_title_end = "\n";

	text_op->document_author_begin = "\nAUTHOR: ";
	text_op->document_author_end = "\n";

	text_op->document_changedate_begin = "\nDATE: ";
	text_op->document_changedate_end = "\n";

	text_op->body_begin = "\n-----------------\n";
	text_op->body_end = "";

	text_op->paragraph_begin = "";
	text_op->paragraph_end = "\n";

	text_op->center_begin = "";
	text_op->center_end = "";

	text_op->justify_begin = ""; 
	text_op->justify_end = "";

	text_op->align_left_begin = ""; 
	text_op->align_left_end = "";

	text_op->align_right_begin = ""; 
	text_op->align_right_end = "";

	text_op->forced_space = " ";
	text_op->line_break = "\n";
	text_op->page_break = "\n";

	text_op->hyperlink_begin = "";
	text_op->hyperlink_end = "";

	text_op->imagelink_begin = "";
	text_op->imagelink_end = "";

	text_op->table_begin = "\n";
	text_op->table_end = "\n";

	text_op->table_row_begin = "";
	text_op->table_row_end = "\n";

	text_op->table_cell_begin = "\t";
	text_op->table_cell_end = "";

	/* Character attributes */
	text_op->font_begin = "";
	text_op->font_end = "";

	text_op->fontsize_begin = "";
	text_op->fontsize_end = "";

	text_op->fontsize8_begin = "";
	text_op->fontsize8_end = "";
	text_op->fontsize10_begin = "";
	text_op->fontsize10_end = "";
	text_op->fontsize12_begin = "";
	text_op->fontsize12_end = "";
	text_op->fontsize14_begin = "";
	text_op->fontsize14_end = "";
	text_op->fontsize18_begin = "";
	text_op->fontsize18_end = "";
	text_op->fontsize24_begin = "";
	text_op->fontsize24_end = "";

	text_op->smaller_begin = "";
	text_op->smaller_end = "";

	text_op->bigger_begin = "";
	text_op->bigger_end = "";

	text_op->foreground_begin = "";
	text_op->foreground_end = "";

	text_op->background_begin = "";
	text_op->background_end = "";

	text_op->bold_begin = "";
	text_op->bold_end = "";

	text_op->italic_begin = "";
	text_op->italic_end = "";

	text_op->underline_begin = "";
	text_op->underline_end = "";

	text_op->dbl_underline_begin = "";
	text_op->dbl_underline_end = "";

	text_op->superscript_begin = "";
	text_op->superscript_end = "";

	text_op->subscript_begin = "";
	text_op->subscript_end = "";

	text_op->strikethru_begin = "";
	text_op->strikethru_end = "";

	text_op->dbl_strikethru_begin = "";
	text_op->dbl_strikethru_end = "";

	text_op->emboss_begin="";
	text_op->emboss_end = "";

	text_op->engrave_begin = "";
	text_op->engrave_end = "";

	text_op->shadow_begin= "";
	text_op->shadow_end= "";

	text_op->outline_begin= "";
	text_op->outline_end= "";

	text_op->expand_begin = "";
	text_op->expand_end = "";

	text_op->pointlist_begin = "\n";
	text_op->pointlist_end = "\n";
	text_op->pointlist_item_begin = "  * ";
	text_op->pointlist_item_end = "\n";

	text_op->numericlist_begin = "\n";
	text_op->numericlist_end = "\n";
	text_op->numericlist_item_begin = "  # ";
	text_op->numericlist_item_end = "\n";

	text_op->simulate_small_caps = TRUE;
	text_op->simulate_all_caps = TRUE;
	text_op->simulate_word_underline = TRUE;

	text_op->ascii_translation_table = ascii_translation_table;

	text_op->ansi_translation_table = upper_translation_table;
	text_op->ansi_first_char = 0x80;
	text_op->ansi_last_char = 0xff;

	text_op->cp437_translation_table = upper_translation_table;
	text_op->cp437_first_char = 0x80;
	text_op->cp437_last_char = 0xff;

	text_op->cp850_translation_table = upper_translation_table;
	text_op->cp850_first_char = 0x80;
	text_op->cp850_last_char = 0xff;

	text_op->mac_translation_table = upper_translation_table;
	text_op->mac_first_char = 0x80;
	text_op->mac_last_char = 0xff;

	text_op->chars.right_quote = "'";
	text_op->chars.left_quote = "`";
	text_op->chars.right_dbl_quote = "''";
	text_op->chars.left_dbl_quote = "``";

	return text_op;
}


