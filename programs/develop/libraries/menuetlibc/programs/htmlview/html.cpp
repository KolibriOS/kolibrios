/*
    Main HTML parser code.
    Copyright (C) 2003 Jarek Pelczar  (jarekp3@wp.pl)

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
*/

#include<stdio.h>
#include"parser.h"
#include<ctype.h>
#include"HTML.h"

CHTMLParser::CHTMLParser(CParser * par,CPageBuffer * buf)
{
 parser=par;
 pgbuf=buf;
 DefaultStyles();
 parser->Init();
}

CHTMLParser::~CHTMLParser()
{
 delete parser;
}

void CHTMLParser::DefaultStyles()
{ 
 text_style.flags=FFLAG_ALIGNLEFT;
 text_style.FontName="system";
 text_style.FontSize=8;
 text_style.color=0;
 text_style.next=NULL;
 page_style.background=0xffffffff;
 page_style.text=0;
 page_style.link=0x008000;
 page_style.alink=0x000080;
 page_style.vlink=0x800000;
 page_style.flags=PFLAG_ALIGNLEFT|PFLAG_RAWMODE;
 pgbuf->Reset();
}

void CHTMLParser::Parse()
{
 char * tokenbuf;
 tokenbuf=new char[1024];
 while(!parser->isEOF())
 {
  parser->SkipWhite();
  if(parser->Look=='<')
  {
   parser->Match('<');
   memset(tokenbuf,0,1024);
   parser->GetToken(tokenbuf,1020);
   parser->Match('>');
  }    
 } 
 delete tokenbuf;
}
