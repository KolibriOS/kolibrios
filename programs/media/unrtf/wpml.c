
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
 * Module name:    wpml
 * Author name:    Zach Smith
 * Create date:    19 Sep 01
 * Purpose:        WPML output module
 * Note:           WPML is my own format; it is a work-in-progress
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
 * Name:	wpml_init
 * Purpose:	Generates an output personality for the WPML format.
 * Args:	None.
 * Returns:	OutputPersonality.
 *=======================================================================*/

OutputPersonality *
wpml_init (void) 
{
	OutputPersonality* wpml_op;

	wpml_op = op_create();

	wpml_op->comment_begin = "<!--";
	wpml_op->comment_end = "-->\n";

	wpml_op->document_begin = "<WPML>";
	wpml_op->document_end = "</WPML>";

	wpml_op->header_begin = "<HEAD>";
	wpml_op->header_end = "</HEAD>";

	wpml_op->document_title_begin = "<TITLE>";
	wpml_op->document_title_end = "</TITLE>";

	wpml_op->document_author_begin = "<AUTHOR>";
	wpml_op->document_author_end = "</AUTHOR>\n";

	wpml_op->document_changedate_begin = "<DATE>";
	wpml_op->document_changedate_end = "</DATE>\n";

	wpml_op->body_begin = "\n<BODY>\n";
	wpml_op->body_end = "</BODY>";

	wpml_op->paragraph_begin = "<LINE>";
	wpml_op->paragraph_end = "</LINE>\n";

	wpml_op->center_begin = "";
	wpml_op->center_end = "";

	wpml_op->justify_begin = ""; 
	wpml_op->justify_end = "";

	wpml_op->align_left_begin = ""; 
	wpml_op->align_left_end = "";

	wpml_op->align_right_begin = ""; 
	wpml_op->align_right_end = "";

	wpml_op->forced_space = " ";
	wpml_op->line_break = "\n";
	wpml_op->page_break = "\n";

	wpml_op->hyperlink_begin = "";
	wpml_op->hyperlink_end = "";

	wpml_op->imagelink_begin = "";
	wpml_op->imagelink_end = "";

	wpml_op->table_begin = "<TABLE>\n";
	wpml_op->table_end = "</TABLE>\n";

	wpml_op->table_row_begin = "<TABLEROW>";
	wpml_op->table_row_end = "</TABLEROW>\n";

	wpml_op->table_cell_begin = "<TABLECELL>";
	wpml_op->table_cell_end = "</TABLECELL>";

	/* Character attributes */

	/* XX: WPML will require that all elements that are now
	 * character attribute strings be converted to functions,
	 * so that a complete font description can be written
	 * each time an attribute begins or ends.
	 */

	wpml_op->font_begin = "<FONT=\"%s\"/>";
	wpml_op->font_end = "";

	wpml_op->fontsize_begin = "";
	wpml_op->fontsize_end = "";

	wpml_op->fontsize8_begin = "";
	wpml_op->fontsize8_end = "";
	wpml_op->fontsize10_begin = "";
	wpml_op->fontsize10_end = "";
	wpml_op->fontsize12_begin = "";
	wpml_op->fontsize12_end = "";
	wpml_op->fontsize14_begin = "";
	wpml_op->fontsize14_end = "";
	wpml_op->fontsize18_begin = "";
	wpml_op->fontsize18_end = "";
	wpml_op->fontsize24_begin = "";
	wpml_op->fontsize24_end = "";

	wpml_op->smaller_begin = "";
	wpml_op->smaller_end = "";

	wpml_op->bigger_begin = "";
	wpml_op->bigger_end = "";

	wpml_op->foreground_begin = "";
	wpml_op->foreground_end = "";

	wpml_op->background_begin = "";
	wpml_op->background_end = "";

	wpml_op->bold_begin = "";
	wpml_op->bold_end = "";

	wpml_op->italic_begin = "";
	wpml_op->italic_end = "";

	wpml_op->underline_begin = "";
	wpml_op->underline_end = "";

	wpml_op->dbl_underline_begin = "";
	wpml_op->dbl_underline_end = "";

	wpml_op->superscript_begin = "";
	wpml_op->superscript_end = "";

	wpml_op->subscript_begin = "";
	wpml_op->subscript_end = "";

	wpml_op->strikethru_begin = "";
	wpml_op->strikethru_end = "";

	wpml_op->dbl_strikethru_begin = "";
	wpml_op->dbl_strikethru_end = "";

	wpml_op->emboss_begin="";
	wpml_op->emboss_end = "";

	wpml_op->engrave_begin = "";
	wpml_op->engrave_end = "";

	wpml_op->shadow_begin= "";
	wpml_op->shadow_end= "";

	wpml_op->outline_begin= "";
	wpml_op->outline_end= "";

	wpml_op->expand_begin = "";
	wpml_op->expand_end = "";

	wpml_op->pointlist_begin = "\n";
	wpml_op->pointlist_end = "\n";
	wpml_op->pointlist_item_begin = "";
	wpml_op->pointlist_item_end = "\n";

	wpml_op->numericlist_begin = "\n";
	wpml_op->numericlist_end = "\n";
	wpml_op->numericlist_item_begin = "";
	wpml_op->numericlist_item_end = "\n";

	wpml_op->simulate_small_caps = TRUE;
	wpml_op->simulate_all_caps = TRUE;
	wpml_op->simulate_word_underline = TRUE;

	wpml_op->ascii_translation_table = ascii_translation_table;

	wpml_op->ansi_translation_table = upper_translation_table;
	wpml_op->ansi_first_char = 0x80;
	wpml_op->ansi_last_char = 0xff;

	wpml_op->cp437_translation_table = upper_translation_table;
	wpml_op->cp437_first_char = 0x80;
	wpml_op->cp437_last_char = 0xff;

	wpml_op->cp850_translation_table = upper_translation_table;
	wpml_op->cp850_first_char = 0x80;
	wpml_op->cp850_last_char = 0xff;

	wpml_op->mac_translation_table = upper_translation_table;
	wpml_op->mac_first_char = 0x80;
	wpml_op->mac_last_char = 0xff;

	wpml_op->chars.right_quote = "'";
	wpml_op->chars.left_quote = "`";
	wpml_op->chars.right_dbl_quote = "''";
	wpml_op->chars.left_dbl_quote = "``";

	return wpml_op;
}


