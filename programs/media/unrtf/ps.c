
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
 * Module name:    ps
 * Author name:    Zach Smith
 * Create date:    18 Sep 01
 * Purpose:        PostScript(TM)-specific output module
 *----------------------------------------------------------------------
 * Changes:
 * 22 Sep 01, tuorfa@yahoo.com: added function-level comment blocks 
 * 23 Sep 01, tuorfa@yahoo.com: added shadow, outline, strikethru, underline
 * 23 Sep 01, tuorfa@yahoo.com: revised PS program to use ISOLatin1Encoding
 * 28 Sep 01, tuorfa@yahoo.com: added support for Helvetica,Courier,Symbol
 *--------------------------------------------------------------------*/




#include <stdio.h>
#include <string.h>
#include "malloc.h"
#include "defs.h"
#include "error.h"
#include "main.h"
#include "output.h"



static char*
ascii [96] = {
	/* 0x20 */ " ", "!", "\"", "#", "$", "\%", "&", "'", 
	/* 0x28 */ "\\(", "\\)", "*", "+", ",", "-", ".", "/", 
	/* 0x30 */ "0", "1", "2", "3", "4", "5", "6", "7", 
	/* 0x38 */ "8", "9", ":", ";", "<", "=", ">", "?", 
	/* 0x40 */ "@", "A", "B", "C", "D", "E", "F", "G", 
	/* 0x48 */ "H", "I", "J", "K", "L", "M", "N", "O", 
	/* 0x50 */ "P", "Q", "R", "S", "T", "U", "V", "W", 
	/* 0x58 */ "X", "Y", "Z", "\\[", "\\\\", "\\]", "^", "_", 
	/* 0x60 */ "`", "a", "b", "c", "d", "e", "f", "g", 
	/* 0x68 */ "h", "i", "j", "k", "l", "m", "n", "o", 
	/* 0x70 */ "p", "q", "r", "s", "t", "u", "v", "w", 
	/* 0x78 */ "x", "y", "z", "{", "|", "}", "~", "", 
};


static char* ansi [] = {
/* 0x80 */ "\\200","\\201","\\202","\\203",
/* 0x84 */ "\\204","\\205","\\206","\\207",
/* 0x88 */ "\\210","\\211","\\212","\\213",
/* 0x8c */ "\\214","\\215","\\216","\\217",
/* 0x90 */ "\\220","\\221","\\222","\\223",
/* 0x94 */ "\\224","\\225","\\226","\\227",
/* 0x98 */ "\\230","\\231","\\232","\\233",
/* 0x9c */ "\\234","\\235","\\236","\\237",
/* 0xa0 */ "\\240","\\241","\\242","\\243",
/* 0xa4 */ "\\244","\\245","\\246","\\247",
/* 0xa8 */ "\\250","\\251","\\252","\\253",
/* 0xac */ "\\254","\\255","\\256","\\257",
/* 0xb0 */ "\\260","\\261","\\262","\\263",
/* 0xb4 */ "\\264","\\265","\\266","\\267",
/* 0xb8 */ "\\270","\\271","\\272","\\273",
/* 0xbc */ "\\274","\\275","\\276","\\277",
/* 0xc0 */ "\\300","\\301","\\302","\\303",
/* 0xc4 */ "\\304","\\305","\\306","\\307",
/* 0xc8 */ "\\310","\\311","\\312","\\313",
/* 0xcc */ "\\314","\\315","\\316","\\317",
/* 0xd0 */ "\\320","\\321","\\322","\\323",
/* 0xd4 */ "\\324","\\325","\\326","\\327",
/* 0xd8 */ "\\330","\\331","\\332","\\333",
/* 0xdc */ "\\334","\\335","\\336","\\337",
/* 0xe0 */ "\\340","\\341","\\342","\\343",
/* 0xe4 */ "\\344","\\345","\\346","\\347",
/* 0xe8 */ "\\350","\\351","\\352","\\353",
/* 0xec */ "\\354","\\355","\\356","\\357",
/* 0xf0 */ "\\360","\\361","\\362","\\363",
/* 0xf4 */ "\\364","\\365","\\366","\\367",
/* 0xf8 */ "\\370","\\371","\\372","\\373",
/* 0xfc */ "\\374","\\375","\\376","\\377",
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




#define PS_END "\
%% --------- \n\n\
  didShowPage not { \n\
    showpage \n\
  } if\n\n\
%%%%EOF\n"




#define PS_START "\
%%%%!PS\n\
%%--------------------------------------------------------------------------\n\
%% GNU UnRTF, a command-line program to convert RTF documents to other formats.\n\
%% Copyright (C) 2000,2001 Zachary Thayer Smith\n\
%%\n\
%% This program is free software; you can redistribute it and/or modify\n\
%% it under the terms of the GNU General Public License as published by\n\
%% the Free Software Foundation; either version 2 of the License, or\n\
%% (at your option) any later version.\n\
%%\n\
%% This program is distributed in the hope that it will be useful,\n\
%% but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
%% GNU General Public License for more details.\n\
%%\n\
%% You should have received a copy of the GNU General Public License\n\
%% along with this program; if not, write to the Free Software\n\
%% Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n\
%%\n\
%% The author is reachable by electronic mail at tuorfa@yahoo.com.\n\
%%--------------------------------------------------------------------------\n\
%%%%EndComments \n\
%%\n\
%%         --------- Note, this PS code is unfinished -------- \n\
%%         --------- Note, this PS code is unfinished -------- \n\
%%         --------- Note, this PS code is unfinished -------- \n\
%%         --------- Note, this PS code is unfinished -------- \n\
%%         --------- Note, this PS code is unfinished -------- \n\
%%\n\
%% ----------- Variables ------------\n\
/fontFamily /Times def\n\
/fontAscent 0 def %% ascent for current font\n\
/fontDescent 0 def %% descent for current font\n\
/lineAscent 0 def \n\
/lineDescent 0 def \n\
/pageWidthInches 8.5 def \n\
/pageHeightInches 11 def \n\
/leftMargin 20 def \n\
/rightMargin 20 def \n\
/topMargin 20 def \n\
/bottomMargin 20 def \n\
/DPI 72 def \n\
/pageWidth pageWidthInches DPI mul def \n\
/rightLimit pageWidth rightMargin sub def \n\
/pageHeight pageHeightInches DPI mul def \n\
/x 0 def \n\
/y 0 def \n\
/bold false def \n\
/italic false def \n\
/underline false def \n\
/overline false def \n\
/intercharSpace 0 def \n\
/strike false def \n\
/outline false def \n\
/shadow false def \n\
/fontSize 12 def \n\
/didBR false def \n\
/didParSkip false def \n\
/didShowPage false def \n\
%%------------------------------------------------------\n\
%% Set up the ISO fonts \n\
%% Times \n\
%% ----- \n\
/Times-Roman findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/TRomanISO exch definefont pop			\n\
/Times-Bold findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/TBoldISO exch definefont pop			\n\
/Times-BoldItalic findfont dup length dict begin {	\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/TBoldItalicISO exch definefont pop			\n\
/Times-Italic findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/TItalicISO exch definefont pop			\n\
%% Courier \n\
%% ----- \n\
/Courier-Roman findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/CRomanISO exch definefont pop			\n\
/Courier-Bold findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/CBoldISO exch definefont pop			\n\
/Courier-BoldItalic findfont dup length dict begin {	\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/CBoldItalicISO exch definefont pop			\n\
/Courier-Italic findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/CItalicISO exch definefont pop			\n\
%% Symbol \n\
%% ----- \n\
/Symbol-Roman findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/SRomanISO exch definefont pop			\n\
/Symbol-Bold findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/SBoldISO exch definefont pop			\n\
/Symbol-BoldItalic findfont dup length dict begin {	\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/SBoldItalicISO exch definefont pop			\n\
/Symbol-Italic findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/SItalicISO exch definefont pop			\n\
%% Helvetica \n\
%% --------- \n\
/Helvetica-Roman findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/HRomanISO exch definefont pop			\n\
/Helvetica-Bold findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/HBoldISO exch definefont pop			\n\
/Helvetica-BoldOblique findfont dup length dict begin {	\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/HBoldItalicISO exch definefont pop			\n\
/Helvetica-Oblique findfont dup length dict begin {		\n\
	1 index /FID ne { def } { pop pop } ifelse	\n\
} forall						\n\
/Encoding ISOLatin1Encoding def 			\n\
currentdict end						\n\
/HItalicISO exch definefont pop			\n\
%% \n\
%% Ideally, before we can draw a line of text, we need to collect all the\n\
%% words that will be on it, just as I do in my Beest HTML viewer, as well\n\
%% as character attributes for each word. But for now, this implementation \n\
%% does not bother. It determines the maximize ascent and descent after\n\
%% drawing the text, not before. XX\n\
%% \n\
%% ----------- Functions ------------\n\
/updateFont { \n\
	/f0 null def \n\
	(Times) fontFamily eq (Times New Roman) fontFamily eq or {		bold { \n\
			italic { /TBoldItalicISO } { /TBoldISO } ifelse \n\
		} { \n\
			italic { /TItalicISO } { /TRomanISO } ifelse \n\
		} \n\
		ifelse \n\
	} if 	(Helvetica) fontFamily eq (Arial) fontFamily eq or { 		bold { \n\
			italic { /HBoldItalicISO } { /HBoldISO } ifelse \n\
		} { \n\
			italic { /HItalicISO } { /HRomanISO } ifelse \n\
		} \n\
		ifelse \n\
	} if	(Courier) fontFamily eq (Courier New) fontFamily eq or {		bold { \n\
			italic { /CBoldItalicISO } { /CBoldISO } ifelse \n\
		} { \n\
			italic { /CItalicISO } { /CRomanISO } ifelse \n\
		} \n\
		ifelse \n\
	} if 	(Symbol) fontFamily eq { 		bold { \n\
			italic { /SBoldItalicISO } { /SBoldISO } ifelse \n\
		} { \n\
			italic { /SItalicISO } { /SRomanISO } ifelse \n\
		} \n\
		ifelse \n\
	} if 	findfont /f0 exch def  \n\
	/bboxBottom f0 /FontBBox get 1 get 1000 div fontSize mul -1 mul def \n\
	/bboxTop    f0 /FontBBox get 3 get 1000 div fontSize mul def \n\
	f0 fontSize scalefont setfont \n\
	lineAscent bboxTop lt { /lineAscent bboxTop def } if  \n\
	lineDescent bboxBottom lt { /lineDescent bboxBottom def } if  \n\
	/fontAscent bboxTop def \n\
	/fontDescent bboxBottom def \n\
} def\n\
/FS { \n\
	/fontSize exch def updateFont \n\
} def \n\
/F { \n\
	/fontFamily exch def updateFont \n\
} def \n\
/resetX { \n\
	/x leftMargin def\n\
} def \n\
/resetY { \n\
	/y pageHeight topMargin sub def \n\
} def \n\
/BR {  \n\
	/oldx x def \n\
	/y y lineAscent lineDescent add sub def  \n\
	resetX \n\
	y bottomMargin lt { \n\
		showpage \n\
		/didShowPage true \n\
		resetY \n\
	} if \n\
	oldx 0 eq didBR and { /didParSkip true def } if \n\
	/didBR true def \n\
	% /lineAscent 0 def \n\
	% /lineDescent 0 def \n\
} def \n\
/P { \n\
	didParSkip not { BR } if \n\
	didParSkip not { BR } if \n\
} \n\
def \n\
/acharpath { \n\
	/acstr exch def pop /acsp exch def 	newpath 	str {		/ch exch def		1 string 0 ch put false charpath		acsp 0 rmoveto	} forall} def \n\
/A { \n\
	/str exch def \n\
	/w str stringwidth pop \n\
		str length intercharSpace mul add \n\
		def \n\
		x w add rightLimit ge { BR } if \n\
	x y moveto \n\
	outline {                                           \n\
		shadow {                                    \n\
			1 -0.1 0 {                        \n\
				/offset exch def \n\
				offset setgray                 \n\
				x offset 3 mul add y offset 3 mul sub moveto 				intercharSpace 0 str acharpath \n\
				%% str false charpath \n\
				fontSize 30 div setlinewidth stroke \n\
			} for                               \n\
			0 setgray \n\
		} {                                         \n\
			intercharSpace 0 str acharpath      \n\
			%% str false charpath \n\
			fontSize 30 div setlinewidth stroke \n\
		} ifelse                                    \n\
	} {                                                 \n\
		shadow {                                    \n\
			1 -0.1 0 {                        \n\
				/offset exch def \n\
				offset setgray                 \n\
				x offset 3 mul add y offset 3 mul sub moveto 				intercharSpace 0 str ashow  \n\
				%% str show \n\
			} for                               \n\
			0 setgray		} {                                         \n\
			intercharSpace 0 str ashow          \n\
			%% str show \n\
		} ifelse                                    \n\
	} ifelse                                            \n\
	strike {							\n\
		newpath fontSize 20 div setlinewidth			\n\
		x y fontAscent 0.32 mul add dup /y2 exch def moveto	\n\
		x w add y2 lineto stroke				\n\
	} if								\n\
	underline {							\n\
		newpath fontSize 20 div setlinewidth			\n\
		x y fontAscent 0.2 mul sub dup /y2 exch def moveto	\n\
		x w add y2 lineto stroke				\n\
	} if								\n\
	overline {							\n\
		%% I don't think RTF supports this, but it can be used later. \n\
		newpath fontSize 20 div setlinewidth			\n\
		x y fontAscent 1.2 mul add dup /y2 exch def moveto	\n\
		x w add y2 lineto stroke				\n\
	} if					\n\
	/x x w add def  			\n\
	/didBR false def 			\n\
	/didShowPage false def 			\n\
} def \n\
\n\
%% These are only binary for now \n\
/X1 { /intercharSpace exch def } def\n\
/X0 { /intercharSpace 0 def } def\n\
/O1 { /outline false def } def\n\
/O0 { /outline false def } def\n\
/H1 { /shadow true def } def\n\
/H0 { /shadow false def } def\n\
/S1 { /strike true def } def\n\
/S0 { /strike false def } def\n\
/B1 { /bold true def updateFont } def\n\
/B0 { /bold false def updateFont } def\n\
/I1 { /italic true def updateFont } def\n\
/I0 { /italic false def updateFont } def\n\
/U1 { /underline true def } def\n\
/U0 { /underline false def } def\n\
updateFont \n\
resetX resetY \n\
\n\
"




/*========================================================================
 * Name:	ps_init
 * Purpose:	Generates an OutputPersonality object for the PostScript(TM)
 *		format.
 * Args:	None.
 * Returns:	OutputPersonality.
 *=======================================================================*/

OutputPersonality *
ps_init (void) 
{
	OutputPersonality* op;

	op = op_create();

	op->comment_begin = "%% ";
	op->comment_end = "\n";

	op->word_begin = "(";
	op->word_end = ")A ";

	op->document_begin = PS_START;
	op->document_end = PS_END;

	op->header_begin = "%% header begin\n";
	op->header_end = "%% header end\n";

	op->document_title_begin = "%%%%Title: ";
	op->document_title_end = "\n";

	op->document_author_begin = "%%%%Creator: ";
	op->document_author_end = "\n";

	op->document_changedate_begin = "%% CHANGED: ";
	op->document_changedate_end = "\n";

	op->body_begin = "\n\n%% ---------- Document Body ------------\n";
	op->body_end = "\n";

	op->paragraph_begin = "P ";
	op->paragraph_end = "\n";

	op->center_begin = "";
	op->center_end = "";

	op->justify_begin = ""; 
	op->justify_end = "";

	op->align_left_begin = ""; 
	op->align_left_end = "";

	op->align_right_begin = ""; 
	op->align_right_end = "";

	op->forced_space = " ";
	op->line_break = "BR\n";
	op->page_break = "\n";

	op->hyperlink_begin = "U1(";
	op->hyperlink_end = ")A U0 ";

	op->imagelink_begin = "";
	op->imagelink_end = "";

	op->table_begin = "\n% TABLE BEGINS (not implemented)\nP\n(TABLE)A BR\n";
	op->table_end = "\n% TABLE ENDS (not implemented)\nP\n";

	op->table_row_begin = "(  )A ";
	op->table_row_end = "( |)A BR\n";

	op->table_cell_begin = "( | )A ";
	op->table_cell_end = "";

	/* Character attributes */
	op->font_begin = "(%s) F ";
	op->font_end = "";

	op->fontsize_begin = "%s FS ";
	op->fontsize_end = "";

	op->smaller_begin = "";
	op->smaller_end = "";

	op->bigger_begin = "";
	op->bigger_end = "";

	op->foreground_begin = "";
	op->foreground_end = "";

	op->background_begin = "";
	op->background_end = "";

	op->bold_begin = "B1 ";
	op->bold_end = "B0 ";

	op->italic_begin = "I1 ";
	op->italic_end = "I0 ";

	op->underline_begin = "U1 ";
	op->underline_end = "U0 ";

	op->dbl_underline_begin = "U1 ";
	op->dbl_underline_end = "U0 ";

	op->superscript_begin = "";
	op->superscript_end = "";

	op->subscript_begin = "";
	op->subscript_end = "";

	op->strikethru_begin = "S1 ";
	op->strikethru_end = "S0 ";

	op->dbl_strikethru_begin = "S1 ";
	op->dbl_strikethru_end = "S0 ";

	op->emboss_begin="";
	op->emboss_end = "";

	op->engrave_begin = "";
	op->engrave_end = "";

	op->shadow_begin= "H1 ";
	op->shadow_end= "H0 ";

	op->outline_begin= "O1 ";
	op->outline_end= "O0 ";

	op->expand_begin = "%s X1 ";
	op->expand_end = "X0 ";

	op->simulate_small_caps = TRUE;
	op->simulate_all_caps = TRUE;
	op->simulate_word_underline = TRUE;

	op->ascii_translation_table = ascii;

	op->ansi_translation_table = ansi;
	op->ansi_first_char = 0x80;
	op->ansi_last_char = 0xff;

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


