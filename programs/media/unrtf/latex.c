
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
 * Module name:    latex
 * Author name:    Zach Smith
 * Create date:    18 Sep 01
 * Purpose:        LaTeX-specific output module
 *----------------------------------------------------------------------
 * Changes:
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks 
 * 23 Sep 01, tuorfa@yahoo.com: fixed accented characters
 *--------------------------------------------------------------------*/




#include <stdio.h>
#include <string.h>
#include "malloc.h"
#include "defs.h"
#include "error.h"
#include "main.h"
#include "output.h"



static char* ascii [96] = {
/* 0x20 */ " ", "!", "''", "\\#", "{\\$}", "\\%", "\\&", "'", 
/* 0x28 */ "(", ")", "{\ast}", "+", ",", "-", ".", "/", 
/* 0x30 */ "0", "1", "2", "3", "4", "5", "6", "7", 
/* 0x38 */ "8", "9", ":", ";", "<", "=", ">", "?", 
/* 0x40 */ "@", "A", "B", "C", "D", "E", "F", "G", 
/* 0x48 */ "H", "I", "J", "K", "L", "M", "N", "O", 
/* 0x50 */ "P", "Q", "R", "S", "T", "U", "V", "W", 
/* 0x58 */ "X", "Y", "Z", "[", "{\\slash}", "]", "{\\caret}", "\\_",
/* 0x60 */ "`", "a", "b", "c", "d", "e", "f", "g", 
/* 0x68 */ "h", "i", "j", "k", "l", "m", "n", "o", 
/* 0x70 */ "p", "q", "r", "s", "t", "u", "v", "w", 
/* 0x78 */ "x", "y", "z", "\\{", "$\\mid$", "\\}", "\\~{ }", "", 
};


static char* ansi [] = {
/* 0x82 */ "?", "?", 
	"?", "{\\ldots}", "{\\dag}", "{\\ddag}", 
	"?", "?", "?", "?",
	"{\\OE}", NULL, NULL, NULL,
/* 0x90 */ NULL,"`","'","``","''","{\\bullet}","--","---",
/* 0x98 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
/* 0xa0 */ "\\:","?","?","{\\pounds}","?","\\Y","?","?",
/* 0xa8 */ "?","{\\copyright}","?","?","?","?","?","?",
/* 0xb0 */ "\\o ", "\\+- ","$^{2}$","$^{3}$","?","?","\\P ","?",
/* 0xb8 */ "?","$^{1}$", "?","?", "\\frac{1}{4}", "\\frac{1}{2}","\\frac{3}{4}",
	"?",
/* 0xc0 */ "\\`{A}","\\'{A}","\\o{A}",
	"\\~{A}","\\\"{A}","?","\\AE ","\\c{C}",
/* 0xc8 */ "\\`{E}","\\'{E}","\\o{E}","\\\"{E}",
	"\\`{I}","\\'{I}","\\o{I}","\\\"{I}",
/* 0xd0 */ "\\ETH ","\\~{N}","\\`{O}","\\'{O}",
	"\\o{O}","\\~{O}","\\\"{O}","\\mult ",
/* 0xd8 */ "?","\\`{U}","\\'{U}","\\o{U}",
	"\\\"{U}","\\'{Y}","\\THORN","?",
/* 0xe0 */ "\\`{a}","\\'{a}","\\o{a}",
	"\\~{a}","\\\"{a}","?","\\ae ","\\c{c}",
/* 0xe8 */ "\\`{e}","\\'{e}","\\o{e}","\\\"{e}",
	"\\`{i}","\\'{i}","\\o{i}","\\\"{i}",
/* 0xf0 */ "\\eth ","\\~{n}","\\`{o}","\\'{o}",
	"\\o{o}","\\~{o}","\\\"{o}","\\div ",
/* 0xf8 */ "\\slash{o}","\\`{u}","\\'{u}","\\o{u}",
	"\\\"{u}","\\'{y}","\\thorn ","\\\"{y}",
};

static char* mac [] = {
	"?",
};

static char* cp437 [] = {
	"?",
};

static char* cp850 [] = {
	"?",
};




/*========================================================================
 * Name:	latex_init
 * Purpose:	Generates the output personality for LaTeX.
 * Args:	None.
 * Returns:	OutputPersonality.
 *=======================================================================*/

OutputPersonality *
latex_init (void) 
{
	OutputPersonality* op;

	op = op_create();

	op->comment_begin = "%% ";
	op->comment_end = "\n";

	op->document_begin = "\\documentclass[11pt]{article}\n";
	op->document_end = "\\end{document}";

	op->header_begin = "";
	op->header_end = "";

	op->document_title_begin = "\\title{";
	op->document_title_end = "}\n";

	op->document_author_begin = "\\author{";
	op->document_author_end = "}\n";

	op->document_changedate_begin = "\\date{";
	op->document_changedate_end = "}\n";

	op->body_begin = "\n\n\\begin{document}\n\\maketitle\n";
	op->body_end = "\\end{document}\n";

	op->paragraph_begin = "\\par\n";
	op->paragraph_end = "";

	op->center_begin = "\\center{\n";
	op->center_end = "}\n";

	op->justify_begin = ""; 
	op->justify_end = "";

	op->align_left_begin = "\\begin{sloppy}\n"; 
	op->align_left_end = "\\end{sloppy}\n";

	op->align_right_begin = ""; 
	op->align_right_end = "";

	op->forced_space = "";
	op->line_break = "\\par\n";
	op->page_break = "\\pagebreak ";

	op->hyperlink_begin = "";
	op->hyperlink_end = "";

	op->imagelink_begin = "";
	op->imagelink_end = "";

	op->table_begin = "\\begin{tabular}{|lllll}\n";
	op->table_end = "\n\\end{tabular}\n";

	op->table_row_begin = "";
	op->table_row_end = "\\hline \\\\\n";

	op->table_cell_begin = "";
	op->table_cell_end = " & ";

	/* Character attributes */
	op->font_begin = "";
	op->font_end = "";

	op->fontsize_begin = "";
	op->fontsize_end = "";

	op->fontsize8_begin = "\\tiny{";
	op->fontsize8_end = "}";
	op->fontsize10_begin = "\\small{";
	op->fontsize10_end = "}";
	op->fontsize12_begin = "\\normalsize{";
	op->fontsize12_end = "}";
	op->fontsize14_begin = "{\\large ";
	op->fontsize14_end = "}";
	op->fontsize18_begin = "{\\Large ";
	op->fontsize18_end = "}";
	op->fontsize24_begin = "{\\LARGE ";
	op->fontsize24_end = "}";
	op->fontsize36_begin = "{\\huge ";
	op->fontsize36_end = "}";
	op->fontsize48_begin = "{\\Huge ";
	op->fontsize48_end = "}";

	op->smaller_begin = "";
	op->smaller_end = "";

	op->bigger_begin = "";
	op->bigger_end = "";

	op->foreground_begin = "";
	op->foreground_end = "";

	op->background_begin = "";
	op->background_end = "";

	op->bold_begin = "{\\bf ";
	op->bold_end = "}";

	op->italic_begin = "{\\it ";
	op->italic_end = "}";

	op->underline_begin = "";
	op->underline_end = "\n";

	op->dbl_underline_begin = "{\\ul ";
	op->dbl_underline_end = "}";

	op->pointlist_begin = "\\begin{itemize}\n";
	op->pointlist_end = "\\end{itemize}\n";
	op->pointlist_item_begin = "\\item ";
	op->pointlist_item_end = "";

	op->numericlist_begin = "\\begin{enumerate}\n";
	op->numericlist_end = "\\end{enumerate}\n";
	op->numericlist_item_begin = "\\item ";
	op->numericlist_item_end = "";

	op->superscript_begin = "$^{";
	op->superscript_end = "}$";

	op->subscript_begin = "$_{";
	op->subscript_end = "}$";

	op->strikethru_begin = "{";
	op->strikethru_end = "}";

	op->dbl_strikethru_begin = "{";
	op->dbl_strikethru_end = "}";

	op->emboss_begin="";
	op->emboss_end = "";

	op->engrave_begin = "";
	op->engrave_end = "";

	op->shadow_begin= "";
	op->shadow_end= "";

	op->small_caps_begin= "\\textsc{";
	op->small_caps_end= "}";

	op->outline_begin= "";
	op->outline_end= "";

	op->expand_begin = "";
	op->expand_end = "";

	op->simulate_small_caps = FALSE;
	op->simulate_all_caps = TRUE;
	op->simulate_word_underline = TRUE;

	op->ascii_translation_table = ascii;

	op->ansi_translation_table = ansi;
	op->ansi_first_char = 0x80;
	op->ansi_last_char = 0x80;

	op->cp437_translation_table = cp437;
	op->cp437_first_char = 0x80;
	op->cp437_last_char = 0x80;

	op->cp850_translation_table = cp850;
	op->cp850_first_char = 0x80;
	op->cp850_last_char = 0x80;

	op->mac_translation_table = mac;
	op->mac_first_char = 0x80;
	op->mac_last_char = 0x80;

	op->chars.right_quote = "'";
	op->chars.left_quote = "`";
	op->chars.right_dbl_quote = "''";
	op->chars.left_dbl_quote = "``";

	return op;
}


