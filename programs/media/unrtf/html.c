
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
 * Module name:    html
 * Author name:    Zach Smith
 * Create date:    18 Sep 01
 * Purpose:        HTML-specific output module
 *----------------------------------------------------------------------
 * Changes:
 * 01 Aug 01, tuorfa@yahoo.com: code moved over from convert.c
 * 03 Aug 01, tuorfa@yahoo.com: removed null entries to save space
 * 08 Aug 01, tuorfa@yahoo.com, gommer@gmx.net: fixed/added some ANSI chars
 * 18 Sep 01, tuorfa@yahoo.com: moved character sets into html.c etc
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks 
 *--------------------------------------------------------------------*/


#include <stdio.h>
#include <string.h>
#include "malloc.h"
#include "defs.h"
#include "error.h"
#include "main.h"
#include "output.h"


static char* ascii [96] = {
	/* 0x20 */ " ", "!", "\"", "#", "$", "%", "&amp;", "'", 
	/* 0x28 */ "(", ")", "*", "+", ",", "-", ".", "/", 
	/* 0x30 */ "0", "1", "2", "3", "4", "5", "6", "7", 
	/* 0x38 */ "8", "9", ":", ";", "&lt;", "=", "&gt;", "?", 
	/* 0x40 */ "@", "A", "B", "C", "D", "E", "F", "G", 
	/* 0x48 */ "H", "I", "J", "K", "L", "M", "N", "O", 
	/* 0x50 */ "P", "Q", "R", "S", "T", "U", "V", "W", 
	/* 0x58 */ "X", "Y", "Z", "[", "\\", "]", "^", "_", 
	/* 0x60 */ "`", "a", "b", "c", "d", "e", "f", "g", 
	/* 0x68 */ "h", "i", "j", "k", "l", "m", "n", "o", 
	/* 0x70 */ "p", "q", "r", "s", "t", "u", "v", "w", 
	/* 0x78 */ "x", "y", "z", "{", "|", "}", "~", "", 
};


static char* ansi [] = {
/* 0x82 */ "&lsquor;", "&fnof;", 
	"&ldquor;", "&hellip;", "&dagger;", "&Dagger;", 
	"&circ;", "&permil;", "&Scaron;", "&lsaquo;",
	"&OElig;", NULL, NULL, NULL,
/* 0x90 */ NULL,"`","'","``","''","&bull;","&ndash;","&mdash;",
/* 0x98 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
/* 0xa0 */ "&nbsp;","&iexcl;","&cent;","&pound;","&curren;","&yen;","&brvbar;","&sect;",
/* 0xa8 */ "&uml;","&copy;","&ordf;","&laquo;","&not;","&shy;","&reg;","&macr;",
/* 0xb0 */ "&deg;", "&plusmn;","&sup2;","&sup3;","&acute;","&micro;","&para;","&middot;",
/* 0xb8 */ "&cedil;","&sup1", "&ordm;","&raquo", "&frac14", "&frac12;","&frac34;","&iquest;",
/* 0xc0 */ "&Agrave;","&Aacute;","&Acirc;","&Atilde;","&Auml;","&Aring;","&AElig;","&Ccedil;",
/* 0xc8 */ "&Egrave;","&Eacute;","&Ecirc;","&Euml;","&Igrave;","&Iacute;","&Icirc;","&Iuml;",
/* 0xd0 */ "&ETH;","&Ntilde;","&Ograve;","&Oacute;","&Ocirc;","&Otilde;","&Ouml;","&times;",
/* 0xd8 */ "&Oslash;","&Ugrave;","&Uacute;","&Ucirc;","&Uuml;","&Yacute;","&THORN;","&szlig;",
/* 0xe0 */ "&agrave;","&aacute;","&acirc;","&atilde;","&auml;","&aring;","&aelig;","&ccedil;",
/* 0xe8 */ "&egrave;","&eacute;","&ecirc;","&euml;","&igrave;","&iacute;","&icirc;","&iuml;",
/* 0xf0 */ "&eth;","&ntilde;","&ograve;","&oacute;","&ocirc;","&otilde;","&ouml;","&divide;",
/* 0xf8 */ "&oslash;","&ugrave;","&uacute;","&ucirc;","&uuml;","&yacute;","&thorn;","&yuml;",
};

static char* mac [] = {
/* 0xa4 */ "&bull;", NULL,NULL,NULL,NULL,NULL,NULL,NULL,
/* 0xb0 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
/* 0xc0 */ NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
/* 0xd0 */ "&mdash;","&ndash;","&ldquo;","&rdquo;","&lquo;","&rquo;",
};

static char* cp437 [] = {
/* 0x80 */ "&ccedil;",
/* 0x81 */ "&uuml;",
/* 0x82 */ "&eacute;",
/* 0x83 */ "&acirc;",
/* 0x84 */ "&auml;",
/* 0x85 */ "&agrave;",
/* 0x86 */ "&aring;",
/* 0x87 */ "&ccedil;",
/* 0x88 */ "&ecirc;",
/* 0x89 */ "&euml;",
/* 0x8a */ "&egrave;",
/* 0x8b */ "&iuml;",
/* 0x8c */ "&icirc;",
/* 0x8d */ "&igrave;",
/* 0x8e */ "&auml;",
/* 0x8f */ "&aring;",
/* 0x90 */ "&eacute;",
/* 0x91 */ "&aelig;",
/* 0x92 */ "&aelig;",
/* 0x93 */ "&ocirc;",
/* 0x94 */ "&ouml;",
/* 0x95 */ "&ograve;",
/* 0x96 */ "&ucirc;",
/* 0x97 */ "&ugrave;",
/* 0x98 */ "&yuml;",
/* 0x99 */ "&ouml;",
/* 0x9a */ "&uuml;",
/* 0x9b */ "&cent;",
/* 0x9c */ "&pound;",
/* 0x9d */ "&yen;",
/* 0x9e */ "&#8359", /* peseta */
/* 0x9f */ "&#402", /* small f with hook */
/* 0xa0 */ "&aacute;",
/* 0xa1 */ "&iacute;",
/* 0xa2 */ "&oacute;",
/* 0xa3 */ "&uacute;",
/* 0xa4 */ "&ntilde;",
/* 0xa5 */ "&ntilde;",
/* 0xa6 */ "&ordf;",
/* 0xa7 */ "&frac14;",
/* 0xa8 */ "&iquest;",
/* 0xa9 */ "&#8976", /* reversed not */
/* 0xaa */ "&not;",
/* 0xab */ "&frac12;",
/* 0xac */ "&raquo;",
/* 0xad */ "&iexcl;",
/* 0xae */ "&laquo;",
/* 0xaf */ "&ordm;",
/* 0xb0 */ "&#9617;", /* light shade */
/* 0xb1 */ "&#9618;", /* med. shade */
/* 0xb2 */ "&#9619;", /* dark shade */
/* 0xb3 */ "&#9474;", /* box-draw light vert. */
/* 0xb4 */ "&#9508;", /* box-draw light vert. + lt. */
/* 0xb5 */ "&#9569;", /* box-draw vert. sgl. + lt. dbl. */
/* 0xb6 */ "&#9570;", /* box-draw vert. dbl. + lt. sgl. */
/* 0xb7 */ "&#9558;", /* box-draw dn. dbl. + lt. sgl. */
/* 0xb8 */ "&#9557;", /* box-draw dn. sgl. + lt. dbl. */
/* 0xb9 */ "&#9571;", /* box-draw dbl. vert. + lt. */
/* 0xba */ "&#9553;", /* box-draw dbl. vert. */
/* 0xbb */ "&#9559;", /* box-draw dbl. dn. + lt. */
/* 0xbc */ "&#9565;", /* box-draw dbl. up + lt. */
/* 0xbd */ "&#9564;", /* box-draw up dbl. + lt. sgl. */
/* 0xbe */ "&#9563;", /* box-draw up sgl. + lt. dbl. */
/* 0xbf */ "&#9488;", /* box-draw light dn. + lt. */
/* 0xc0 */ "&#9492;", /* box-draw light up + rt. */
/* 0xc1 */ "&#9524;", /* box-draw light up + horiz. */
/* 0xc2 */ "&#9516;", /* box-draw light dn. + horiz. */
/* 0xc3 */ "&#9500;", /* box-draw light vert. + rt. */
/* 0xc4 */ "&#9472;", /* box-draw light horiz. */
/* 0xc5 */ "&#9532;", /* box-draw light vert. + horiz. */
/* 0xc6 */ "&#9566;", /* box-draw vert. sgl. + rt. dbl. */
/* 0xc7 */ "&#9567;", /* box-draw vert. dbl. + rt. sgl. */
/* 0xc8 */ "&#9562;", /* box-draw dbl. up + rt. */
/* 0xc9 */ "&#9556;", /* box-draw dbl. dn. + rt. */
/* 0xca */ "&#9577;", /* box-draw dbl. up + horiz. */
/* 0xcb */ "&#9574;", /* box-draw dbl. dn. + horiz. */
/* 0xcc */ "&#9568;", /* box-draw dbl. vert. + rt. */
/* 0xcd */ "&#9552;", /* box-draw dbl. horiz. */
/* 0xce */ "&#9580;", /* box-draw dbl. vert. + horiz. */
/* 0xcf */ "&#9575;", /* box-draw up sgl. + horiz. dbl. */
/* 0xd0 */ "&#9576;", /* box-draw up dbl. + horiz. sgl. */
/* 0xd1 */ "&#9572;", /* box-draw dn. sgl. + horiz. dbl. */
/* 0xd2 */ "&#9573;", /* box-draw dn. dbl. + horiz. sgl. */
/* 0xd3 */ "&#9561;", /* box-draw up dbl. + rt. sgl. */
/* 0xd4 */ "&#9560;", /* box-draw up sgl. + rt. dbl. */
/* 0xd5 */ "&#9554;", /* box-draw dn. sgl. + rt. dbl. */
/* 0xd6 */ "&#9555;", /* box-draw dn. dbl. + rt. sgl. */
/* 0xd7 */ "&#9579;", /* box-draw vert. dbl. + horiz. sgl. */
/* 0xd8 */ "&#9578;", /* box-draw vert. sgl. + horiz. dbl. */
/* 0xd9 */ "&#9496;", /* box-draw light up + lt. */
/* 0xda */ "&#9484;", /* box-draw light dn. + rt. */
/* 0xdb */ "&#9608;", /* full block */
/* 0xdc */ "&#9604;", /* lower 1/2 block */
/* 0xdd */ "&#9612;", /* lt. 1/2 block */
/* 0xde */ "&#9616;", /* rt. 1/2 block */
/* 0xdf */ "&#9600;", /* upper 1/2 block */
/* 0xe0 */ "&#945;", /* greek small alpha */
/* 0xe1 */ "&szlig;",
/* 0xe2 */ "&#915;", /* greek cap gamma */
/* 0xe3 */ "&#960;", /* greek small pi */
/* 0xe4 */ "&#931;", /* greek cap sigma */
/* 0xe5 */ "&#963;", /* greek small sigma */
/* 0xe6 */ "&micro;",
/* 0xe7 */ "&#964;", /* greek small tau */
/* 0xe8 */ "&#934;", /* greek cap phi */
/* 0xe9 */ "&#920;", /* greek cap theta */
/* 0xea */ "&#937;", /* greek cap omega */
/* 0xeb */ "&#948;", /* greek small delta */
/* 0xec */ "&#8734;", /* inf. */
/* 0xed */ "&#966;", /* greek small phi */
/* 0xee */ "&#949;", /* greek small epsilon */
/* 0xef */ "&#8745;", /* intersect */
/* 0xf0 */ "&#8801;", /* identical */
/* 0xf1 */ "&plusmn;",
/* 0xf2 */ "&#8805;", /* greater-than or equal to */
/* 0xf3 */ "&#8804;", /* less-than or equal to */
/* 0xf4 */ "&#8992;", /* top 1/2 integral */
/* 0xf5 */ "&#8993;", /* bottom 1/2 integral */
/* 0xf6 */ "&divide;",
/* 0xf7 */ "&#8776;", /* almost = */
/* 0xf8 */ "&plus;",
/* 0xf9 */ "&#8729;", /* bullet op */
/* 0xfa */ "&middot;",
/* 0xfb */ "&#8730;", /* sqrt */
/* 0xfc */ "&#8319;", /* super-script small n */
/* 0xfd */ "&sup2;",
/* 0xfe */ "&#9632;", /* black square */
/* 0xff */ "&nbsp;",
};

static char* cp850 [] = {
/* 0x80 */  "&ccedil;",
/* 0x81 */  "&uuml;",
/* 0x82 */  "&eacute;",
/* 0x83 */  "&acirc;",
/* 0x84 */  "&auml;",
/* 0x85 */  "&agrave;",
/* 0x86 */  "&aring;",
/* 0x87 */  "&ccedil;",
/* 0x88 */  "&ecirc;",
/* 0x89 */  "&euml;",
/* 0x8a */  "&egrave;",
/* 0x8b */  "&iuml;",
/* 0x8c */  "&icirc;",
/* 0x8d */  "&igrave;",
/* 0x8e */  "&auml;",
/* 0x8f */  "&aring;",
/* 0x90 */  "&eacute;",
/* 0x91 */  "&aelig;",
/* 0x92 */  "&aelig;",
/* 0x93 */  "&ocirc;",
/* 0x94 */  "&ouml;",
/* 0x95 */  "&ograve;",
/* 0x96 */  "&ucirc;",
/* 0x97 */  "&ugrave;",
/* 0x98 */  "&yuml;",
/* 0x99 */  "&ouml;",
/* 0x9a */  "&uuml;",
/* 0x9b */  "&oslash;",
/* 0x9c */  "&pound;",
/* 0x9d */  "&oslash;",
/* 0x9e */  "&times;",
/* 0x9f */  "&#402;", /* small f with hook */
/* 0xa0 */  "&aacute;",
/* 0xa1 */  "&iacute;",
/* 0xa2 */  "&oacute;",
/* 0xa3 */  "&uacute;",
/* 0xa4 */  "&ntilde;",
/* 0xa5 */  "&ntilde;",
/* 0xa6 */  "&ordf;",
/* 0xa7 */  "&frac14;",
/* 0xa8 */  "&iquest;",
/* 0xa9 */  "&reg;",
/* 0xaa */  "&not;",
/* 0xab */  "&frac12;",
/* 0xac */  "&raquo;",
/* 0xad */  "&iexcl;",
/* 0xae */  "&laquo;",
/* 0xaf */  "&ordm;",
/* 0xb0 */  "&#9617;", /* light shade */
/* 0xb1 */  "&#9618;", /* med. shade */
/* 0xb2 */  "&#9619;", /* dark shade */
/* 0xb3 */  "&#9474;", /* box-draw light vert. */
/* 0xb4 */  "&#9508;", /* box-draw light vert. + lt. */
/* 0xb5 */  "&aacute;",
/* 0xb6 */  "&acirc;",
/* 0xb7 */  "&agrave;",
/* 0xb8 */  "&copy;",
/* 0xb9 */  "&#9571;", /* box-draw dbl. vert. + lt. */
/* 0xba */  "&#9553;", /* box-draw dbl. vert. */
/* 0xbb */  "&#9559;", /* box-draw dbl. dn. + lt. */
/* 0xbc */  "&#9565;", /* box-draw dbl. up + lt. */
/* 0xbd */  "&cent;",
/* 0xbe */  "&yen;",
/* 0xbf */  "&#9488;", /* box-draw light dn. + lt. */
/* 0xc0 */  "&#9492;", /* box-draw light up + rt. */
/* 0xc1 */  "&#9524;", /* box-draw light up + horiz. */
/* 0xc2 */  "&#9516;", /* box-draw light dn. + horiz. */
/* 0xc3 */  "&#9500;", /* box-draw light vert. + rt. */
/* 0xc4 */  "&#9472;", /* box-draw light horiz. */
/* 0xc5 */  "&#9532;", /* box-draw light vert. + horiz. */
/* 0xc6 */  "&atilde;",
/* 0xc7 */  "&atilde;",
/* 0xc8 */  "&#9562;", /* box-draw dbl. up + rt. */
/* 0xc9 */  "&#9556;", /* box-draw dbl. dn. + rt. */
/* 0xca */  "&#9577;", /* box-draw dbl. up + horiz. */
/* 0xcb */  "&#9574;", /* box-draw dbl. dn. + horiz. */
/* 0xcc */  "&#9568;", /* box-draw dbl. vert. + rt. */
/* 0xcd */  "&#9552;", /* box-draw dbl. horiz. */
/* 0xce */  "&#9580;", /* box-draw dbl. vert. + horiz. */
/* 0xcf */  "&curren;",
/* 0xd0 */  "&eth;",
/* 0xd1 */  "&eth;",
/* 0xd2 */  "&ecirc;",
/* 0xd3 */  "&euml;",
/* 0xd4 */  "&egrave;",
/* 0xd5 */  "&#305;", /* small dotless i */
/* 0xd6 */  "&iacute;",
/* 0xd7 */  "&icirc;",
/* 0xd8 */  "&iuml;",
/* 0xd9 */  "&#9496;", /* box-draw light up + lt. */
/* 0xda */  "&#9484;", /* box-draw light dn. + rt. */
/* 0xdb */  "&#9608;", /* full-block */
/* 0xdc */  "&#9604;", /* lower 1/2 block */
/* 0xdd */  "&brvbar;",
/* 0xde */  "&igrave;",
/* 0xdf */  "&#9600;", /* upper 1/2 block */
/* 0xe0 */  "&oacute;",
/* 0xe1 */  "&szlig;",
/* 0xe2 */  "&ocirc;",
/* 0xe3 */  "&ograve;",
/* 0xe4 */  "&otilde;",
/* 0xe5 */  "&otilde;",
/* 0xe6 */  "&micro;",
/* 0xe7 */  "&thorn;",
/* 0xe8 */  "&thorn;",
/* 0xe9 */  "&uacute;",
/* 0xea */  "&ucirc;",
/* 0xeb */  "&ugrave;",
/* 0xec */  "&yacute;",
/* 0xed */  "&yacute;",
/* 0xee */  "&macr;",
/* 0xef */  "&acute;",
/* 0xf0 */  "&shy;",
/* 0xf1 */  "&plusmn;",
/* 0xf2 */  "&#8215;", /* dbl. lowline */
/* 0xf3 */  "&frac34;",
/* 0xf4 */  "&para;",
/* 0xf5 */  "&sect;",
/* 0xf6 */  "&divide;",
/* 0xf7 */  "&cedil;",
/* 0xf8 */  "&plus;",
/* 0xf9 */  "&uml;",
/* 0xfa */  "&middot;",
/* 0xfb */  "&sup1;",
/* 0xfc */  "&sup3;",
/* 0xfd */  "&sup2;",
/* 0xfe */  "&#9632;", /* black square */
/* 0xff */  "&nbsp;",
};




/*========================================================================
 * Name:	html_init
 * Purpose:	Generates the HTML output personality.
 * Args:	None.
 * Returns:	OutputPersonality.
 *=======================================================================*/

OutputPersonality *
html_init (void) 
{
	OutputPersonality* op;

	op = op_create();

	op->comment_begin = "<!--- ";
	op->comment_end = " --->\n";

	op->document_begin = "<html>\n";
	op->document_end = "</html>\n";

	op->header_begin = "<head>\n";
	op->header_end = "</head>\n";

	op->document_title_begin = "<title>";
	op->document_title_end = "</title>\n";

	op->document_author_begin = "<!--author: ";
	op->document_author_end = "--->\n";

	op->document_changedate_begin = "<!--changed: ";
	op->document_changedate_end = "--->\n";

	op->body_begin = "<body>";
	op->body_end = "</body>\n";

	op->paragraph_begin = "<p>";
	op->paragraph_end = "</p>\n";

	op->center_begin = "<center>";
	op->center_end = "</center>\n";

	op->justify_begin = "<div align=justify>\n"; 
	op->justify_end = "</div>\n";

	op->align_left_begin = "<div align=left>\n"; 
	op->align_left_end = "</div>\n";

	op->align_right_begin = "<div align=right>\n"; 
	op->align_right_end = "</div>\n";

	op->forced_space = "&nbsp;";
	op->line_break = "<br>\n";
	op->page_break = "<p><hr><p>\n";

	op->hyperlink_begin = "<a href=\"";
	op->hyperlink_end = "\">hyperlink</a>";

	op->imagelink_begin = "<img src=\"";
	op->imagelink_end = "\">";

	op->table_begin = "<table border=2>\n";
	op->table_end = "</table>\n";

	op->table_row_begin = "<tr>";
	op->table_row_end = "</tr>\n";

	op->table_cell_begin = "<td>";
	op->table_cell_end = "</td>\n";

	/* Character attributes */
	op->font_begin = "<font face=\"%s\">";
	op->font_end = "</font>";

	op->fontsize_begin = "<span style=\"font-size:%spt\">";
	op->fontsize_end = "</span>";

	op->fontsize8_begin = "<font size=1>";
	op->fontsize8_end = "</font>";
	op->fontsize10_begin = "<font size=2>";
	op->fontsize10_end = "</font>";
	op->fontsize12_begin = "<font size=3>";
	op->fontsize12_end = "</font>";
	op->fontsize14_begin = "<font size=4>";
	op->fontsize14_end = "</font>";
	op->fontsize18_begin = "<font size=5>";
	op->fontsize18_end = "</font>";
	op->fontsize24_begin = "<font size=6>";
	op->fontsize24_end = "</font>";

	op->smaller_begin = "<small>";
	op->smaller_end = "</small>";

	op->bigger_begin = "<big>";
	op->bigger_end = "</big>";

	op->foreground_begin = "<font color=\"%s\">";
	op->foreground_end = "</font>";

	op->background_begin = "<span style=\"background:%s\">";
	op->background_end = "</span>";

	op->bold_begin = "<b>";
	op->bold_end = "</b>";

	op->italic_begin = "<i>";
	op->italic_end = "</i>";

	op->underline_begin = "<u>";
	op->underline_end = "</u>";

	op->dbl_underline_begin = "<u>";
	op->dbl_underline_end = "</u>";

	op->superscript_begin = "<sup>";
	op->superscript_end = "</sup>";

	op->subscript_begin = "<sub>";
	op->subscript_end = "</sub>";

	op->strikethru_begin = "<s>";
	op->strikethru_end = "</s>";

	op->dbl_strikethru_begin = "<s>";
	op->dbl_strikethru_end = "</s>";

	op->emboss_begin="<span style=\"background:gray\"><font color=black>";
	op->emboss_end = "</font></span>";

	op->engrave_begin = "<span style=\"background:gray\"><font color=navyblue>";
	op->engrave_end = "</font></span>";

	op->shadow_begin= "<span style=\"background:gray\">";
	op->shadow_end= "</span>";

	op->outline_begin= "<span style=\"background:gray\">";
	op->outline_end= "</span>";

	op->expand_begin = "<span style=\"letter-spacing: %s\">";
	op->expand_end = "</span>";

	op->pointlist_begin = "<ol>\n";
	op->pointlist_end = "</ol>\n";
	op->pointlist_item_begin = "<li>";
	op->pointlist_item_end = "</li>\n";

	op->numericlist_begin = "<ul>\n";
	op->numericlist_end = "</ul>\n";
	op->numericlist_item_begin = "<li>";
	op->numericlist_item_end = "</li>\n";

	op->simulate_small_caps = TRUE;
	op->simulate_all_caps = TRUE;
	op->simulate_word_underline = TRUE;

	op->ascii_translation_table = ascii;

	op->ansi_translation_table = ansi;
	op->ansi_first_char = 0x82;
	op->ansi_last_char = 0xff;

	op->cp437_translation_table = cp437;
	op->cp437_first_char = 0x80;
	op->cp437_last_char = 0xff;

	op->cp850_translation_table = cp850;
	op->cp850_first_char = 0x80;
	op->cp850_last_char = 0xff;

	op->mac_translation_table = mac;
	op->mac_first_char = 0xa4;
	op->mac_last_char = 0xd5;

	op->chars.right_quote = "'";
	op->chars.left_quote = "`";
	op->chars.right_dbl_quote = "''";
	op->chars.left_dbl_quote = "``";

	return op;
}


