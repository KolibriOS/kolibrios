
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
 * Module name:    vt
 * Author name:    Zach Smith
 * Create date:    19 Sep 01
 * Purpose:        text output with VT100 escape codes module
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
 * Name:	vt_init
 * Purpose:	Generates an output personality for the VT100 text format.
 * Args:	None.
 * Returns:	OutputPersonality.
 *=======================================================================*/

OutputPersonality *
vt_init (void) 
{
	OutputPersonality* vt_op;

	vt_op = op_create();

	vt_op->comment_begin = "### ";
	vt_op->comment_end = "\n";

	vt_op->document_begin = "";
	vt_op->document_end = "";

	vt_op->header_begin = "";
	vt_op->header_end = "";

	vt_op->document_title_begin = "\nTITLE: ";
	vt_op->document_title_end = "\n";

	vt_op->document_author_begin = "\nAUTHOR: ";
	vt_op->document_author_end = "\n";

	vt_op->document_changedate_begin = "\nDATE: ";
	vt_op->document_changedate_end = "\n";

	vt_op->body_begin = "\n-----------------\n";
	vt_op->body_end = "";

	vt_op->paragraph_begin = "";
	vt_op->paragraph_end = "\n";

	vt_op->center_begin = "";
	vt_op->center_end = "";

	vt_op->justify_begin = ""; 
	vt_op->justify_end = "";

	vt_op->align_left_begin = ""; 
	vt_op->align_left_end = "";

	vt_op->align_right_begin = ""; 
	vt_op->align_right_end = "";

	vt_op->forced_space = " ";
	vt_op->line_break = "\n";
	vt_op->page_break = "\n";

	vt_op->hyperlink_begin = "";
	vt_op->hyperlink_end = "";

	vt_op->imagelink_begin = "";
	vt_op->imagelink_end = "";

	vt_op->table_begin = "\n";
	vt_op->table_end = "\n";

	vt_op->table_row_begin = "";
	vt_op->table_row_end = "\n";

	vt_op->table_cell_begin = "\t";
	vt_op->table_cell_end = "";

	/* Character attributes */
	vt_op->font_begin = "";
	vt_op->font_end = "";

	vt_op->fontsize_begin = "";
	vt_op->fontsize_end = "";

	vt_op->fontsize8_begin = "";
	vt_op->fontsize8_end = "";
	vt_op->fontsize10_begin = "";
	vt_op->fontsize10_end = "";
	vt_op->fontsize12_begin = "";
	vt_op->fontsize12_end = "";
	vt_op->fontsize14_begin = "";
	vt_op->fontsize14_end = "";
	vt_op->fontsize18_begin = "";
	vt_op->fontsize18_end = "";
	vt_op->fontsize24_begin = "";
	vt_op->fontsize24_end = "";

	vt_op->smaller_begin = "";
	vt_op->smaller_end = "";

	vt_op->bigger_begin = "";
	vt_op->bigger_end = "";

	vt_op->foreground_begin = "";
	vt_op->foreground_end = "";

	vt_op->background_begin = "";
	vt_op->background_end = "";

	vt_op->bold_begin = "\033[7m";
	vt_op->bold_end = "\033[m";

	vt_op->italic_begin = "\033[7m";
	vt_op->italic_end = "\033[m";

	vt_op->underline_begin = "\033[4m";
	vt_op->underline_end = "\033[m";

	vt_op->dbl_underline_begin = "";
	vt_op->dbl_underline_end = "";

	vt_op->superscript_begin = "";
	vt_op->superscript_end = "";

	vt_op->subscript_begin = "";
	vt_op->subscript_end = "";

	vt_op->strikethru_begin = "";
	vt_op->strikethru_end = "";

	vt_op->dbl_strikethru_begin = "";
	vt_op->dbl_strikethru_end = "";

	vt_op->emboss_begin="";
	vt_op->emboss_end = "";

	vt_op->engrave_begin = "";
	vt_op->engrave_end = "";

	vt_op->shadow_begin= "";
	vt_op->shadow_end= "";

	vt_op->outline_begin= "";
	vt_op->outline_end= "";

	vt_op->expand_begin = "";
	vt_op->expand_end = "";

	vt_op->pointlist_begin = "\n";
	vt_op->pointlist_end = "\n";
	vt_op->pointlist_item_begin = "  * ";
	vt_op->pointlist_item_end = "\n";

	vt_op->numericlist_begin = "\n";
	vt_op->numericlist_end = "\n";
	vt_op->numericlist_item_begin = "  # ";
	vt_op->numericlist_item_end = "\n";

	vt_op->simulate_small_caps = TRUE;
	vt_op->simulate_all_caps = TRUE;
	vt_op->simulate_word_underline = TRUE;

	vt_op->ascii_translation_table = ascii_translation_table;

	vt_op->ansi_translation_table = upper_translation_table;
	vt_op->ansi_first_char = 0x80;
	vt_op->ansi_last_char = 0xff;

	vt_op->cp437_translation_table = upper_translation_table;
	vt_op->cp437_first_char = 0x80;
	vt_op->cp437_last_char = 0xff;

	vt_op->cp850_translation_table = upper_translation_table;
	vt_op->cp850_first_char = 0x80;
	vt_op->cp850_last_char = 0xff;

	vt_op->mac_translation_table = upper_translation_table;
	vt_op->mac_first_char = 0x80;
	vt_op->mac_last_char = 0xff;

	vt_op->chars.right_quote = "'";
	vt_op->chars.left_quote = "`";
	vt_op->chars.right_dbl_quote = "''";
	vt_op->chars.left_dbl_quote = "``";

	return vt_op;
}


