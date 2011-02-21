/*
    Generic parser class for HTML viewer.
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

CParser::CParser()
{
 IsEOF=false;
 Look=0;
}

CParser::~CParser()
{
}

char CParser::GetChar()
{
 return 0;
}

void CParser::Init()
{
 GetChar();
}

void CParser::SkipWhite()
{
 if(IsEOF) return;
 while(Look==' ' || Look=='\t' || Look=='\n' || Look=='\r')
  GetChar();
}

int CParser::Match(char c)
{
 if(IsEOF) return 0;
 SkipWhite();
 if(Look==c) return 1;
 return 0;
}

static inline int hex2dec(char c)
{
 if(c>='0'&&c<='9') return c-'0';
 if(c>='a'&&c<='f') return c-'a'+10;
 if(c>='A'&&c<='F') return c-'A'+10;
 return 0;
}

unsigned long CParser::__GetNum()
{
 unsigned long __ret=0;
 SkipWhite();
 while(isdigit(Look))
 {
  __ret=(__ret*10)+(Look-'0');
  GetChar();
 }
 return __ret;
}

unsigned long CParser::__GetHexNum()
{
 unsigned long __ret=0;
 SkipWhite();
 while(isxdigit(Look))
 {
  __ret=(__ret<<4)+hex2dec(Look);
  GetChar();
 }
 return __ret;
}

unsigned long CParser::__GetOctNum()
{
 unsigned long __ret=0;
 SkipWhite();
 while(Look>='0' && Look<='7')
 {
  __ret=(__ret<<3)+(Look-'0');
  GetChar();
 }
 return __ret;
}

/* This routine simply calls previously defined functions.
   So the number may be (of course in HTML) :
    number example		class
    #1BADB002			hex
    #0x1BADB002			hex
    $0x1BADB002			hex
    0xF002			hex
    03312			octal
    32768			decimal
 Note:
  We don't have to remember leading zeroes because they always give 0
  so there isn't anything to complain about.
*/
unsigned long CParser::GetNum()
{
 if(Look=='#')
 {
  Match('#');
  if(Look=='0')
  {
   GetChar();
   if(Look=='x')
   {
    GetChar();
    return __GetHexNum();
   }
  }
  return __GetHexNum();
 }
 if(Look=='$')
 {
  Match('$');
  return __GetHexNum();
 }
 if(Look=='0')
 {
  GetChar();
  if(Look=='x')
  {
   GetChar();
   return __GetHexNum();
  }
  return __GetOctNum();
 }
 return __GetNum();
}

int CParser::GetToken(char * tokbuf,int maxlen)
{
 int p;
 if(!tokbuf || !maxlen) return 0;
 if(IsEOF) return 0;
 SkipWhite();
 if(!isalpha(Look) && Look!='_') return 0;
 p=0;
 while(maxlen-- && (isalpha(Look) || isdigit(Look) || Look=='_'))
 {
  *tokbuf++=Look;
  GetChar();
  p++;
 }
 return p;
}
