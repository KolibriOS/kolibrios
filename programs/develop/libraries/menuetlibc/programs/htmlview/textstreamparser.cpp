/*
    Parser class for memory stream derived from CParser class.
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

CTextStreamParser::CTextStreamParser(char * Stm,int stm_size):CParser()
{
 __Stm=Stm;
 __Size=stm_size;
 __Pos=0;
}

CTextStreamParser::~CTextStreamParser()
{
 this->CParser::~CParser();
}

char CTextStreamParser::GetChar()
{
 unsigned char c;
 if(IsEOF) return 0;
 if(__Pos>=__Size)
 {
  IsEOF=true;
  return 0;
 }
 c=*(unsigned char *)(__Stm+__Pos);
 __Pos++;
 Look=c;
 return c;
}
